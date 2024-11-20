
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

namespace engine
{
  using namespace DirectX;

  // Based on multiple training projects:
  // https://github.com/jpvanoosten/LearningDirectX12/tree/main/samples
  // https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop
  
  enum root_parameter_type : int
  {
    object_data = 0,
    frame_data,
    lights,
    materials,
    textures,
    num
  };
  
  void fforward_pass::init()
  {
    fpass_base::init();
    
    uint32_t back_buffer_count = context->back_buffer_count;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fdevice& device = fapplication::get_instance()->device;
    
    // Create frame data CBV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fconst_buffer buffer;
      device.create_const_buffer(heap, sizeof(fframe_data), buffer, std::format("CBV frame: back buffer {}", i).c_str());
      frame_data.emplace_back(buffer);
    }

    // Create light and material data SRV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device.create_shader_resource_buffer(heap, sizeof(flight_properties) * MAX_LIGHTS, buffer, std::format("SRV lights: back buffer {}", i).c_str());
      lights_data.emplace_back(buffer);
    }
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device.create_shader_resource_buffer(heap, sizeof(fmaterial_properties) * MAX_MATERIALS, buffer, std::format("SRV materials: back buffer {}", i).c_str());
      materials_data.emplace_back(buffer);
    }

    // Create first texture SRV (handles only)
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();
    device.create_texture_buffer(heap, default_texture, "default");
    
    // Set up graphics pipeline
    graphics_pipeline.reserve_parameters(root_parameter_type::num);
    graphics_pipeline.add_constant_parameter(root_parameter_type::object_data, 0, 0, static_cast<uint32_t>(sizeof(fobject_data)), D3D12_SHADER_VISIBILITY_VERTEX);
    graphics_pipeline.add_constant_buffer_view_parameter(root_parameter_type::frame_data, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_shader_respurce_view_parameter(root_parameter_type::lights, 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_shader_respurce_view_parameter(root_parameter_type::materials, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_descriptor_table_parameter(root_parameter_type::textures, 2, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_static_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
    graphics_pipeline.bind_pixel_shader(pixel_shader_asset.get()->resource.blob);
    graphics_pipeline.bind_vertex_shader(vertex_shader_asset.get()->resource.blob);
    graphics_pipeline.setup_formats({ DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT);
    graphics_pipeline.setup_input_layout({
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    });
    graphics_pipeline.init("Forward pass");
  }

  void fforward_pass::init_size_dependent(bool cleanup)
  {
    fdevice& device = fapplication::get_instance()->device;

    if(cleanup)
    {
      context->main_descriptor_heap->remove(color.srv.index);
      context->rtv_descriptor_heap->remove(color.rtv.index);
      context->dsv_descriptor_heap->remove(depth.dsv.index);
    }
    device.create_frame_buffer(context->main_descriptor_heap, context->rtv_descriptor_heap, color, context->width, context->height, graphics_pipeline.get_rtv_format(0), D3D12_RESOURCE_STATE_COPY_SOURCE, "Forward pass");
    device.create_depth_stencil(context->dsv_descriptor_heap, depth, context->width, context->height, graphics_pipeline.get_depth_format(), D3D12_RESOURCE_STATE_COPY_SOURCE, "Forward pass");
  }
  
  void fforward_pass::draw(fgraphics_command_list* command_list)
  {
    fpass_base::draw(command_list);
    
    fdevice& device = fapplication::get_instance()->device;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();
    
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();
    
    command_list->resource_barrier(color.com.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    command_list->resource_barrier(depth.com.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    command_list->clear_render_target(color);
    command_list->clear_depth_stencil(depth);
    command_list->set_render_targets(color, &depth);
        
    graphics_pipeline.bind_command_list(command_list_com);

    command_list_com->SetDescriptorHeaps(1, heap->com.GetAddressOf());

    const uint32_t back_buffer_index = context->back_buffer_index;
    const uint32_t N = static_cast<uint32_t>(scene_acceleration.h_meshes.size());
    
    // Process object data
    for(uint32_t i = 0; i < N; i++)
    {
      fobject_data& object_data = scene_acceleration.object_buffer[i];
      const hstatic_mesh* sm = scene_acceleration.h_meshes[i];
      object_data.is_selected = context->selected_object == sm ? 1 : 0;
      object_data.object_id = fmath::uint32_to_colorf(sm->get_hash());
    }

    // Process frame data CBV
    {
      fframe_data data;
      data.camera_position = XMFLOAT4(context->scene->camera_config.location.e);
      data.ambient_light = context->scene->ambient_light_color;
      data.show_ambient = show_ambient;
      data.show_diffuse = show_diffuse;
      data.show_emissive = show_emissive;
      data.show_normals = show_normals;
      data.show_object_id = show_object_id;
      data.show_specular = show_specular;
      frame_data[back_buffer_index].upload(&data);
    }

    // Process light and material SRVs
    {
      lights_data[back_buffer_index].upload(scene_acceleration.lights_buffer.data());
      materials_data[back_buffer_index].upload(scene_acceleration.materials_buffer.data());
    }

    // Process texture SRVs
    {
      const uint32_t num_textures_in_scene = static_cast<uint32_t>(scene_acceleration.a_textures.size());

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
            device.create_texture_buffer(heap, texture, texture->get_display_name().c_str());
            
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
        command_list->upload_vertex_buffer(amesh, std::format("{}{}", mesh_name, asset_name).c_str());
        command_list->upload_index_buffer(amesh, std::format("{}{}", mesh_name, asset_name).c_str());
      }
    }
    
    // Draw
    for(uint32_t i = 0; i < N; i++)
    {
      const fstatic_mesh_resource& smrs = context->scene->scene_acceleration.h_meshes[i]->mesh_asset_ptr.get()->resource;

      command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, sizeof(fobject_data)/4, &scene_acceleration.object_buffer[i], 0);
      command_list_com->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, frame_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::lights, lights_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::materials, materials_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::textures, default_texture->gpu_resource.srv.gpu_handle);
      command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
    }
    
    command_list->resource_barrier(color.com.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
    command_list->resource_barrier(depth.com.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE);
  }
}