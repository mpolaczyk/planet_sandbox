#include "stdafx.h"

#include "forward_pass.h"

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

  // Based on multiple training projects:
  // https://github.com/jpvanoosten/LearningDirectX12/tree/main/samples
  // https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop

  namespace
  {
    enum root_parameter_type : int
    {
      object_data = 0,
      frame_data,
      lights,
      materials,
      textures,
      num
    };

    DXGI_FORMAT rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depth_format = DXGI_FORMAT_D32_FLOAT;
  }

  void fforward_pass::init_shaders()
  {
    vertex_shader_asset.set_name("forward");
    pixel_shader_asset.set_name("forward");
  }

  void fforward_pass::init_pipeline()
  {
    fpass_base::init_pipeline();
    const uint32_t num_textures = context->scene->scene_acceleration.get_num_textures();
    graphics_pipeline->reserve_parameters(root_parameter_type::num);
    graphics_pipeline->add_constant_parameter(root_parameter_type::object_data, 0, 0, fmath::to_uint32(sizeof(fobject_data)), D3D12_SHADER_VISIBILITY_VERTEX);
    graphics_pipeline->add_constant_buffer_view_parameter(root_parameter_type::frame_data, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_shader_resource_view_parameter(root_parameter_type::lights, 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_shader_resource_view_parameter(root_parameter_type::materials, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_descriptor_table_parameter(root_parameter_type::textures, 2, 0, num_textures, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_static_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
    graphics_pipeline->setup_formats(1, &rtv_format, depth_format);
    graphics_pipeline->init("Forward pass");
  }

  void fforward_pass::init_size_independent_resources()
  {
    uint32_t back_buffer_count = context->back_buffer_count;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fdevice* device = fapplication::get_instance()->device.get();
    
    // Create frame data CBV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fconst_buffer buffer;
      device->create_const_buffer(heap, fmath::to_uint32(sizeof(fframe_data)), buffer, fstring_tools::format("CBV frame: back buffer {}", i).c_str());
      frame_data.emplace_back(buffer);
    }

    // Create light and material data SRV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device->create_shader_resource_buffer(heap, sizeof(flight_properties) * MAX_LIGHTS, buffer, fstring_tools::format("SRV lights: back buffer {}", i).c_str());
      lights_data.emplace_back(buffer);
    }
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device->create_shader_resource_buffer(heap, sizeof(fmaterial_properties) * MAX_MATERIALS, buffer, fstring_tools::format("SRV materials: back buffer {}", i).c_str());
      materials_data.emplace_back(buffer);
    }
  }

  void fforward_pass::init_size_dependent_resources(bool cleanup)
  {
    fdevice* device = fapplication::get_instance()->device.get();

    if(cleanup)
    {
      context->main_descriptor_heap->remove(color.srv.index);
      context->rtv_descriptor_heap->remove(color.rtv.index);
      context->dsv_descriptor_heap->remove(depth.dsv.index);
    }
    color.release();
    device->create_frame_buffer(context->main_descriptor_heap, context->rtv_descriptor_heap, &color, context->width, context->height, rtv_format, D3D12_RESOURCE_STATE_RENDER_TARGET, "Forward pass");
    depth.release();
    device->create_depth_stencil(context->dsv_descriptor_heap, &depth, context->width, context->height, depth_format, D3D12_RESOURCE_STATE_DEPTH_WRITE, "Forward pass");
  }
  
  void fforward_pass::draw(frenderer_context* in_context, fgraphics_command_list* command_list)
  {    
    fpass_base::draw(in_context, command_list);
    
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();
    const uint32_t back_buffer_index = context->back_buffer_index;
    
    const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    command_list->clear_render_target(&color, black);
    command_list->clear_depth_stencil(&depth);
    command_list->set_render_targets1(&color, &depth);

    // Process frame data CBV
    {
      fframe_data data;
      data.camera_position = XMFLOAT4(context->scene->camera.location.e);
      data.ambient_light = context->scene->ambient_light_color;
      data.height = context->height;
      data.width = context->width;
      frame_data[back_buffer_index].upload(&data);
    }

    // Process light and material SRVs
    {
      lights_data[back_buffer_index].upload(scene_acceleration.get_light_properties());
      materials_data[back_buffer_index].upload(scene_acceleration.get_material_properties());
    }
    
    update_vertex_and_index_buffers(command_list);
    
    upload_all_textures_once(command_list);
    
    // Draw
    command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::lights, lights_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::materials, materials_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::textures, get_textures_gpu_handle());
    for(uint32_t i = 0; i < scene_acceleration.get_num_meshes(); i++)
    {
      const fstatic_mesh_resource& smrs = context->scene->scene_acceleration.get_mesh(i)->mesh_asset_ptr.get()->resource;
      command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, fmath::to_uint32(sizeof(fobject_data))/4, scene_acceleration.get_object_data(i), 0);
      command_list_com->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, frame_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
    }
  }
}