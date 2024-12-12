#include <DirectXColors.h>
#include "d3d12.h"
#include "d3dx12/d3dx12_barriers.h"
#include "d3dx12/d3dx12_core.h"
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_resource_helpers.h"

#include "renderer/command_list.h"

#include "assets/texture.h"
#include "core/application.h"
#include "engine/log.h"
#include "engine/string_tools.h"
#include "hittables/static_mesh.h"
#include "math/math.h"
#include "math/vertex_data.h"
#include "renderer/dx12_lib.h"
#include "renderer/device.h"

namespace engine
{
  void fgraphics_command_list::resource_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const
  {
    const CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, state_before, state_after);
    com->ResourceBarrier(1, &resource_barrier);

    //LOG_INFO("Barrier for {} from {} to {}", fdx12::get_resource_name(resource), static_cast<uint32_t>(state_before), static_cast<uint32_t>(state_after));
  }

  void fgraphics_command_list::set_render_targets1(ftexture_resource* render_target, const ftexture_resource* dsv) const
  {
    ftexture_resource* render_target_array[1] = {render_target};
    set_render_targets(1, render_target_array, dsv);
  }

  void fgraphics_command_list::set_render_targets(uint32_t num_render_targets, ftexture_resource** render_targets, const ftexture_resource* dsv) const
  {
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtv_handles;
    for(uint32_t i = 0; i < num_render_targets; i++)
    {
      rtv_handles.push_back(render_targets[i]->rtv.cpu_descriptor_handle);
    }
    com->OMSetRenderTargets(fmath::to_uint32(rtv_handles.size()), rtv_handles.data(), FALSE, dsv ? &dsv->dsv.cpu_descriptor_handle : nullptr);
  }

  void fgraphics_command_list::set_viewport(uint32_t width, uint32_t height) const
  {
    CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    com->RSSetViewports(1, &viewport);
  }

  void fgraphics_command_list::set_scissor(uint32_t width, uint32_t height) const
  {
    CD3DX12_RECT scissor_rect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
    com->RSSetScissorRects(1, &scissor_rect);
  }

  void fgraphics_command_list::clear_render_target(const ftexture_resource* rtv, const float color[4]) const
  {
    com->ClearRenderTargetView(rtv->rtv.cpu_descriptor_handle, color, 0, nullptr);
  }

  void fgraphics_command_list::clear_depth_stencil(const ftexture_resource* dsv) const
  {
    com->ClearDepthStencilView(dsv->dsv.cpu_descriptor_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
  }

  void fgraphics_command_list::upload_buffer_resource(uint32_t buffer_size, const void* in_buffer, ComPtr<ID3D12Resource>& out_upload_intermediate, ComPtr<ID3D12Resource>& out_gpu_resource) const
  {
    fdevice* device = fapplication::get_instance()->device.get();

    if(in_buffer)
    {
      device->create_buffer_resource(buffer_size, out_gpu_resource);
      device->create_upload_resource(buffer_size, out_upload_intermediate);

      D3D12_SUBRESOURCE_DATA data;
      data.pData = in_buffer;
      data.RowPitch = buffer_size;
      data.SlicePitch = data.RowPitch;

      UpdateSubresources(com.Get(), out_gpu_resource.Get(), out_upload_intermediate.Get(), 0, 0, 1, &data);
    }
  }

  void fgraphics_command_list::upload_vertex_buffer(astatic_mesh* mesh, const char* name) const
  {
    fstatic_mesh_resource& smr = mesh->resource;
    const uint32_t vertex_list_size = fmath::to_uint32(mesh->vertex_list.size() * sizeof(fvertex_data));

    upload_buffer_resource(vertex_list_size, mesh->vertex_list.data(), smr.vertex_buffer_upload, smr.vertex_buffer);

    smr.vertex_buffer_view.BufferLocation = smr.vertex_buffer->GetGPUVirtualAddress();
    smr.vertex_buffer_view.SizeInBytes = vertex_list_size;
    smr.vertex_buffer_view.StrideInBytes = fmath::to_uint32(sizeof(fvertex_data));
    smr.vertex_num = fmath::to_uint32(mesh->vertex_list.size());

#if BUILD_DEBUG
    DX_SET_NAME(smr.vertex_buffer, "Vertex buffer {}", name)
    DX_SET_NAME(smr.vertex_buffer_upload, "Vertex buffer upload {}", name)
#endif
    
    resource_barrier(smr.vertex_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    mesh->is_resource_online = true;
  }

  void fgraphics_command_list::upload_index_buffer(astatic_mesh* mesh, const char* name) const
  {
    fstatic_mesh_resource& smr = mesh->resource;
    const uint32_t face_list_size = fmath::to_uint32(mesh->face_list.size() * sizeof(fface_data));

    upload_buffer_resource(face_list_size, mesh->face_list.data(), smr.index_buffer_upload, smr.index_buffer);

    smr.index_buffer_view.BufferLocation = smr.index_buffer->GetGPUVirtualAddress();
    smr.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    smr.index_buffer_view.SizeInBytes = face_list_size;

#if BUILD_DEBUG
    DX_SET_NAME(smr.index_buffer, "Index buffer {}", name)
    DX_SET_NAME(smr.index_buffer_upload, "Index buffer upload {}", name)
#endif
    
    resource_barrier(smr.index_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

    mesh->is_resource_online = true;
  }

  void fgraphics_command_list::upload_texture(atexture* texture_asset) const
  {
    ftexture_resource& gpur = texture_asset->gpu_resource;

    // Copy data to the intermediate upload heap and then schedule a copy 
    // from the upload heap to the Texture2D.
    D3D12_SUBRESOURCE_DATA texture_data = {};
    texture_data.pData = texture_asset->is_hdr
                           ? reinterpret_cast<void*>(texture_asset->data_hdr.data())
                           : texture_asset->data_ldr.data();
    texture_data.RowPitch = texture_asset->width * texture_asset->channels * texture_asset->element_size;
    texture_data.SlicePitch = texture_data.RowPitch * texture_asset->height;

    UpdateSubresources(com.Get(), gpur.com.Get(), gpur.upload_com.Get(), 0, 0, 1, &texture_data);

    resource_barrier(gpur.com.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    texture_asset->is_online = true;
  }

  fresource_barrier_scope::fresource_barrier_scope(fgraphics_command_list* in_command_list, ID3D12Resource* in_resource, D3D12_RESOURCE_STATES in_before, D3D12_RESOURCE_STATES in_after)
    : command_list(in_command_list), resource(in_resource), before(in_before), after(in_after)
  {
    command_list->resource_barrier(resource, before, after);
  }

  fresource_barrier_scope::~fresource_barrier_scope()
  {
    command_list->resource_barrier(resource, after, before);
  }
}