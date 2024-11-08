
#include <DirectXColors.h>
#include "d3d12.h"
#include "d3dx12/d3dx12_barriers.h"
#include "d3dx12/d3dx12_core.h"
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_resource_helpers.h"

#include "renderer/command_list.h"

#include "assets/texture.h"
#include "core/application.h"
#include "math/vertex_data.h"
#include "renderer/render_state.h"

namespace engine
{

  void fgraphics_command_list::resource_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const
  {
    const CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, state_before, state_after);
    com->ResourceBarrier(1, &resource_barrier);
  }
  
  void fgraphics_command_list::set_render_targets(ID3D12DescriptorHeap* dsv_descriptor_heap, ID3D12DescriptorHeap* rtv_descriptor_heap, int back_buffer_index) const
  {
    fdevice& device = fapplication::instance->device;
    uint32_t rtv_descriptor_size = device.com.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), back_buffer_index, rtv_descriptor_size);
    D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
    com->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);
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

  void fgraphics_command_list::clear_render_target(ID3D12DescriptorHeap* rtv_descriptor_heap, uint32_t back_buffer_index) const
  {
    fdevice& device = fapplication::instance->device;
    const uint32_t descriptor_size = device.com.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), back_buffer_index, descriptor_size);
    com->ClearRenderTargetView(handle, DirectX::Colors::LightSlateGray, 0, nullptr);
  }

  void fgraphics_command_list::clear_depth_stencil(ID3D12DescriptorHeap* dsv_descriptor_heap) const
  {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
    com->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
  }

  void fgraphics_command_list::upload_buffer_resource(uint64_t buffer_size, const void* in_buffer, ComPtr<ID3D12Resource>& out_upload_intermediate, ComPtr<ID3D12Resource>& out_gpu_resource) const
  {
    fdevice& device = fapplication::instance->device;

    if (in_buffer)
    {
      device.create_buffer_resource(buffer_size, out_gpu_resource);
      device.create_upload_resource(buffer_size, out_upload_intermediate);
      
      D3D12_SUBRESOURCE_DATA data;
      data.pData = in_buffer;
      data.RowPitch = static_cast<uint32_t>(buffer_size);
      data.SlicePitch = data.RowPitch;

      UpdateSubresources(com.Get(), out_gpu_resource.Get(), out_upload_intermediate.Get(), 0, 0, 1, &data);
    }
  }
  
  void fgraphics_command_list::upload_vertex_buffer(fstatic_mesh_render_state& out_render_state) const
  {
    const uint64_t vertex_list_size = out_render_state.vertex_list.size() * sizeof(fvertex_data);
    
    upload_buffer_resource(vertex_list_size, out_render_state.vertex_list.data(), out_render_state.vertex_buffer_upload, out_render_state.vertex_buffer);
    
    out_render_state.vertex_buffer_view.BufferLocation = out_render_state.vertex_buffer->GetGPUVirtualAddress();
    out_render_state.vertex_buffer_view.SizeInBytes = static_cast<uint32_t>(vertex_list_size);
    out_render_state.vertex_buffer_view.StrideInBytes = sizeof(fvertex_data);

    resource_barrier(out_render_state.vertex_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    
    out_render_state.is_resource_online = true;
  }

  void fgraphics_command_list::upload_index_buffer(fstatic_mesh_render_state& out_render_state) const
  {
    const uint64_t face_list_size = out_render_state.face_list.size() * sizeof(fface_data); 

    upload_buffer_resource(face_list_size, out_render_state.face_list.data(), out_render_state.index_buffer_upload, out_render_state.index_buffer);
    
    out_render_state.index_buffer_view.BufferLocation = out_render_state.index_buffer->GetGPUVirtualAddress();
    out_render_state.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    out_render_state.index_buffer_view.SizeInBytes = static_cast<uint32_t>(face_list_size);

    resource_barrier(out_render_state.index_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    
    out_render_state.is_resource_online = true;
  }

  void fgraphics_command_list::upload_texture(atexture* texture_asset) const
  {
    ftexture_resource& gpur = texture_asset->gpu_resource;
    
    // Copy data to the intermediate upload heap and then schedule a copy 
    // from the upload heap to the Texture2D.
    D3D12_SUBRESOURCE_DATA texture_data = {};
    texture_data.pData = texture_asset->is_hdr ? reinterpret_cast<void*>(texture_asset->data_hdr.data()) : texture_asset->data_ldr.data();
    texture_data.RowPitch = texture_asset->width * texture_asset->channels * texture_asset->element_size;
    texture_data.SlicePitch = texture_data.RowPitch * texture_asset->height;

    UpdateSubresources(com.Get(), gpur.resource.Get(), gpur.resource_upload.Get(), 0, 0, 1, &texture_data);
    
    resource_barrier(gpur.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    texture_asset->is_online = true;
  }
  
}