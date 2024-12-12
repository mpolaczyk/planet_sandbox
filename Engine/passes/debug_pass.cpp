
#include <format>

#include "d3dx12/d3dx12.h"

#include "debug_pass.h"

#include "core/application.h"
#include "core/window.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "math/math.h"
#include "renderer/aligned_structs.h"
#include "renderer/command_list.h"
#include "renderer/render_context.h"
#include "renderer/scene_acceleration.h"
#include "renderer/gpu_resources.h"
#include "renderer/device.h"

namespace engine
{
  using namespace DirectX;

  namespace
  {
    ALIGNED_STRUCT_BEGIN(fdebug_frame_data)
    {
      XMFLOAT4 camera_position; // 16
    };
    ALIGNED_STRUCT_END(fdebug_frame_data)
    
    enum root_parameter_type : int
    {
      object_data = 0,
      frame_data,
      num
    };

    DXGI_FORMAT rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM;
  }

  void fdebug_pass::init_shaders()
  {
    vertex_shader_asset.set_name("debug");
    pixel_shader_asset.set_name("debug");
  }

  void fdebug_pass::init_pipeline()
  {
    fpass_base::init_pipeline();

    graphics_pipeline->reserve_parameters(root_parameter_type::num);
    graphics_pipeline->add_constant_parameter(root_parameter_type::object_data, 0, 0, fmath::to_uint32(sizeof(fobject_data)), D3D12_SHADER_VISIBILITY_VERTEX);
    graphics_pipeline->add_constant_buffer_view_parameter(root_parameter_type::frame_data, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->setup_formats(1, &rtv_format, DXGI_FORMAT_UNKNOWN);
    graphics_pipeline->setup_blend(0, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD);
    graphics_pipeline->init("Debug pass");
  }

  void fdebug_pass::init_size_independent_resources()
  {
    uint32_t back_buffer_count = context->back_buffer_count;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fdevice* device = fapplication::get_instance()->device.get();
    
    // Create frame data CBV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fconst_buffer buffer;
      device->create_const_buffer(heap, fmath::to_uint32(sizeof(fdebug_frame_data)), buffer, std::format("CBV frame: back buffer {}", i).c_str());
      frame_data.emplace_back(buffer);
    }
  }

  void fdebug_pass::init_size_dependent_resources(bool cleanup)
  {
    // Nothing to do here
  }

  void fdebug_pass::draw(frenderer_context* in_context, fgraphics_command_list* command_list)
  {
    fpass_base::draw(in_context, command_list);
    
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();
    
    command_list->set_render_targets1(blend_on, nullptr);
    graphics_pipeline->bind_command_list(command_list_com);
    command_list_com->SetDescriptorHeaps(1, heap->com.GetAddressOf());

    const uint32_t back_buffer_index = context->back_buffer_index;
    
    // Process frame data CBV
    {
      fdebug_frame_data data;
      data.camera_position = XMFLOAT4(context->scene->camera.location.e);
      frame_data[back_buffer_index].upload(&data);
    }
    
    // Find selected object
    if(!context->selected_object) return;
    uint32_t index = 0;
    for(uint32_t i = 0; i < fmath::to_uint32(scene_acceleration.h_meshes.size()); i++)
    {
      if(context->selected_object == scene_acceleration.h_meshes[i])
      {
        index = i;
        break;
      }
    }

    // Update vertex and index buffers
    hstatic_mesh* hmesh = scene_acceleration.h_meshes[index];
    astatic_mesh* amesh = hmesh->mesh_asset_ptr.get();
    if(!amesh->is_resource_online)
    {
      std::string mesh_name = hmesh->get_display_name();
      std::string asset_name = hmesh->mesh_asset_ptr.get()->name;
      command_list->upload_vertex_buffer(amesh, std::format("{} {}", mesh_name, asset_name).c_str());
      command_list->upload_index_buffer(amesh, std::format("{} {}", mesh_name, asset_name).c_str());
    }

    // Draw
    const fstatic_mesh_resource& smrs = context->scene->scene_acceleration.h_meshes[index]->mesh_asset_ptr.get()->resource;
    command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, fmath::to_uint32(sizeof(fobject_data))/4, &scene_acceleration.object_buffer[index], 0);
    command_list_com->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, frame_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
    command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
    command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
  }
}