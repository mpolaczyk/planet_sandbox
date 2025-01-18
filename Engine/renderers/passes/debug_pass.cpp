#include "stdafx.h"

#include "debug_pass.h"

#include "hittables/scene.h"
#include "hittables/static_mesh.h"

#include "assets/mesh.h"

#include "engine/window.h"
#include "engine/math/math.h"
#include "engine/string_tools.h"
#include "engine/renderer/aligned_structs.h"
#include "engine/renderer/command_list.h"
#include "engine/renderer/render_context.h"
#include "engine/renderer/scene_acceleration.h"
#include "engine/renderer/gpu_resources.h"
#include "engine/renderer/device.h"
#include "engine/renderer/graphics_pipeline.h"

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
    froot_signature sig;
    sig.reserve_parameters(root_parameter_type::num);
    sig.add_constant_parameter(root_parameter_type::object_data, 0, 0, fmath::to_uint32(sizeof(fobject_data)), D3D12_SHADER_VISIBILITY_VERTEX);
    sig.add_constant_buffer_view_parameter(root_parameter_type::frame_data, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    pipeline->raster = sig;
    pipeline->setup_formats(1, &rtv_format, DXGI_FORMAT_UNKNOWN);
    pipeline->setup_blend(0, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD);
    pipeline->init("Debug pass");
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
      device->create_const_buffer(heap, fmath::to_uint32(sizeof(fdebug_frame_data)), buffer, fstring_tools::append("CBV frame: back buffer ", i).c_str());
      frame_data.emplace_back(buffer);
    }
  }

  void fdebug_pass::init_size_dependent_resources(bool cleanup)
  {
    // Nothing to do here
  }

  void fdebug_pass::draw(frenderer_context* in_context, fcommand_list* command_list)
  {
    fpass_base::draw(in_context, command_list);
    
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();
    
    command_list->set_render_targets1(blend_on, nullptr);

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
    for(uint32_t i = 0; i < scene_acceleration.get_num_meshes(); i++)
    {
      if(context->selected_object == scene_acceleration.get_mesh(i))
      {
        index = i;
        break;
      }
    }

    update_vertex_and_index_buffers(command_list);

    // Draw
    const fstatic_mesh_resource& smrs = context->scene->scene_acceleration.get_mesh(index)->mesh_asset_ptr.get()->resource;
    command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, fmath::to_uint32(sizeof(fobject_data))/4, scene_acceleration.get_object_data(index), 0);
    command_list_com->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, frame_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
    command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
    command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
  }
}