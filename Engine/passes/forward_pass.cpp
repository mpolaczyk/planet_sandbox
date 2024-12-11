
#include "forward_pass.h"

#include <format>

#include "d3dx12/d3dx12.h"
#include "d3dx12/d3dx12_root_signature.h"

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
    graphics_pipeline->reserve_parameters(root_parameter_type::num);
    graphics_pipeline->add_constant_parameter(root_parameter_type::object_data, 0, 0, fmath::to_uint32(sizeof(fobject_data)), D3D12_SHADER_VISIBILITY_VERTEX);
    graphics_pipeline->add_constant_buffer_view_parameter(root_parameter_type::frame_data, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_shader_resource_view_parameter(root_parameter_type::lights, 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_shader_resource_view_parameter(root_parameter_type::materials, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->add_descriptor_table_parameter(root_parameter_type::textures, 2, 0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
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
      device->create_const_buffer(heap, fmath::to_uint32(sizeof(fframe_data)), buffer, std::format("CBV frame: back buffer {}", i).c_str());
      frame_data.emplace_back(buffer);
    }

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

    // Create first texture SRV (handles only)
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();
    if(!default_texture->is_online)
    {
      device->create_texture_buffer(heap, default_texture, "default");
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
    
    fdevice* device = fapplication::get_instance()->device.get();
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();
    
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();

    command_list->clear_render_target(&color);
    command_list->clear_depth_stencil(&depth);
    
    command_list->set_render_targets1(&color, &depth);
        
    graphics_pipeline->bind_command_list(command_list_com);

    command_list_com->SetDescriptorHeaps(1, heap->com.GetAddressOf());

    const uint32_t back_buffer_index = context->back_buffer_index;
    const uint32_t N = fmath::to_uint32(scene_acceleration.h_meshes.size());
    
    // Process object data
    for(uint32_t i = 0; i < N; i++)
    {
      fobject_data& object_data = scene_acceleration.object_buffer[i];
      const hstatic_mesh* sm = scene_acceleration.h_meshes[i];
      object_data.is_selected = context->selected_object == sm ? 1 : 0;
    }

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
      lights_data[back_buffer_index].upload(scene_acceleration.lights_buffer.data());
      materials_data[back_buffer_index].upload(scene_acceleration.materials_buffer.data());
    }

    // Process texture SRVs
    {
      const uint32_t num_textures_in_scene = fmath::to_uint32(scene_acceleration.a_textures.size());

      // Upload default texture first
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
    
    // Update vertex and index buffers
    for(uint32_t i = 0; i < N; i++)
    {
      hstatic_mesh* hmesh = scene_acceleration.h_meshes[i];
      astatic_mesh* amesh = hmesh->mesh_asset_ptr.get();
      if(!amesh->is_resource_online)
      {
        std::string mesh_name = hmesh->get_display_name();
        std::string asset_name = hmesh->mesh_asset_ptr.get()->name;
        command_list->upload_vertex_buffer(amesh, std::format("{} {}", mesh_name, asset_name).c_str());
        command_list->upload_index_buffer(amesh, std::format("{} {}", mesh_name, asset_name).c_str());
      }
    }
    
    // Draw
    for(uint32_t i = 0; i < N; i++)
    {
      const fstatic_mesh_resource& smrs = context->scene->scene_acceleration.h_meshes[i]->mesh_asset_ptr.get()->resource;
      command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, fmath::to_uint32(sizeof(fobject_data))/4, &scene_acceleration.object_buffer[i], 0);
      command_list_com->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, frame_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::lights, lights_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::materials, materials_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::textures, default_texture->gpu_resource.srv.gpu_descriptor_handle);
      command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
    }
  }
}