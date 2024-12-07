#include "deferred_lighting_pass.h"

#include <vector>

#include "core/application.h"
#include "core/core.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "math/math.h"
#include "renderer/aligned_structs.h"
#include "renderer/command_list.h"
#include "renderer/render_context.h"
#include "renderer/scene_acceleration.h"
#include "renderer/device.h"

namespace engine
{
  using namespace DirectX;

  namespace
  {
    enum root_parameter_type : int
    {
      frame_data = 0,
      lights,
      materials,
      gbuffer_position,
      gbuffer_normal,
      gbuffer_uv,
      gbuffer_material_id,
      textures,   // TODO! gbuffer position, gbuffer normal, gbuffer uv, gbuffer material_id, default texture, scene textures (unbound)
      num
    };

    DXGI_FORMAT rtv_formats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
    const char* rtv_names[1] = { "color"};
  }

  void fdeferred_lighting_pass::init_pipeline()
  {
    fpass_base::init_pipeline();
    graphics_pipeline->reserve_parameters(root_parameter_type::num);
    graphics_pipeline->add_constant_parameter(root_parameter_type::frame_data, 0, 0, fmath::to_uint32(sizeof(fframe_data)), D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_shader_resource_view_parameter(root_parameter_type::lights, 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_shader_resource_view_parameter(root_parameter_type::materials, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    // TODO GBuffer as one table, or maybe all textures...
    graphics_pipeline->add_descriptor_table_parameter(root_parameter_type::gbuffer_position, 2, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_descriptor_table_parameter(root_parameter_type::gbuffer_normal, 3, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_descriptor_table_parameter(root_parameter_type::gbuffer_uv, 4, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_descriptor_table_parameter(root_parameter_type::gbuffer_material_id, 5, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_descriptor_table_parameter(root_parameter_type::textures, 6, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_static_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
    graphics_pipeline->setup_formats(1, rtv_formats, DXGI_FORMAT_UNKNOWN);
    graphics_pipeline->init("Deferred lighting pass");
  }

  void fdeferred_lighting_pass::init_size_independent_resources()
  {
    uint32_t back_buffer_count = context->back_buffer_count;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fdevice* device = fapplication::get_instance()->device.get();

    // Create light and material data SRV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device->create_shader_resource_buffer(heap, sizeof(flight_properties) * MAX_LIGHTS, buffer, std::format("SRV lights: back buffer {}", i).c_str());
      lights_data.emplace_back(buffer);
    }
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device->create_shader_resource_buffer(heap, sizeof(fmaterial_properties) * MAX_MATERIALS, buffer, std::format("SRV materials: back buffer {}", i).c_str());
      materials_data.emplace_back(buffer);
    }
    
    // Load and create default texture resource
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();
    device->create_texture_buffer(heap, default_texture, "default");
    
    // Load and create quad mesh
    quad_asset.set_name("plane");
    quad_asset.get();
  }

  void fdeferred_lighting_pass::init_size_dependent_resources(bool cleanup)
  {
    fdevice* device = fapplication::get_instance()->device.get();
  
    if(cleanup)
    {
      context->main_descriptor_heap->remove(color.srv.index);
      context->rtv_descriptor_heap->remove(color.rtv.index);  
    }
    device->create_frame_buffer(context->main_descriptor_heap, context->rtv_descriptor_heap,
      &color, context->width, context->height, rtv_formats[0], D3D12_RESOURCE_STATE_RENDER_TARGET, rtv_names[0]);
  }
  
  void fdeferred_lighting_pass::draw(fgraphics_command_list* command_list)
  {
    fpass_base::draw(command_list);
    
    fdevice* device = fapplication::get_instance()->device.get();
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();
    
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();

    // Clear and setup
    command_list->clear_render_target(&color);
    command_list->set_render_targets1(&color, nullptr);
    graphics_pipeline->bind_command_list(command_list_com);
    command_list_com->SetDescriptorHeaps(1, heap->com.GetAddressOf());

    const uint32_t back_buffer_index = context->back_buffer_index;

    // Process frame data constants
    frame_data.camera_position = XMFLOAT4(context->scene->camera.location.e);
    frame_data.ambient_light = context->scene->ambient_light_color;

    // Process light and material SRVs
    {
      lights_data[back_buffer_index].upload(scene_acceleration.lights_buffer.data());
      materials_data[back_buffer_index].upload(scene_acceleration.materials_buffer.data());
    }

    // Process texture SRVs
    {
      const uint32_t num_textures_in_scene = fmath::to_uint32(scene_acceleration.a_textures.size());

      // Upload default texture first
      // TODO move to init
      if(!default_texture->is_online)
      {
        command_list->upload_texture(default_texture);
      }
      
      // Upload other textures
      for(uint32_t i = 0; i < MAX_TEXTURES-1; i++)
      {
        if(i < num_textures_in_scene && scene_acceleration.a_textures[i] != default_texture)
        {
          atexture* texture = scene_acceleration.a_textures[i];
          if(!texture->is_online)
          {
            device->create_texture_buffer(heap, texture, texture->get_display_name().c_str());
            
            command_list->upload_texture(texture);
          }
        }
      }
    }
    
    // Upload quad mesh
    // TODO move to init
    astatic_mesh* quad_mesh = quad_asset.get();
    if(!quad_mesh->is_resource_online)
    {
      command_list->upload_vertex_buffer(quad_mesh, "quad");
      command_list->upload_index_buffer(quad_mesh, "quad");
    }

    // Draw
    const fstatic_mesh_resource& smrs = quad_mesh->resource;
    command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::frame_data, sizeof(fframe_data)/4, &frame_data, 0);
    command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::lights, lights_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::materials, materials_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_position, position->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_normal, normal->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_uv, uv->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_material_id, material_id->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::textures, default_texture->gpu_resource.srv.gpu_descriptor_handle);
    command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
    command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
    command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
  }
}