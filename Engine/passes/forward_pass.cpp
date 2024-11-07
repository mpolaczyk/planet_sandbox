
#include "forward_pass.h"

#include <format>

#include "d3dx12/d3dx12.h"
#include "d3dx12/d3dx12_root_signature.h"

#include "core/application.h"
#include "core/exceptions.h"
#include "core/window.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "math/math.h"
#include "renderer/dx12_lib.h"
#include "renderer/render_state.h"
#include "renderer/aligned_structs.h"
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
    fdevice& device = fapplication::instance->device;

    //fdx12::create_texture2d_resource(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_RENDER_TARGET, output);
    //fdx12::create_render_target(device, swap_chain, rtv_descriptor_heap, back_buffer_count, rtv);
    //fdx12::create_depth_stencil(device, dsv_descriptor_heap, width, height, back_buffer_count, dsv);

    // Create frame data CBV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      frame_data.emplace_back(fconst_buffer::create(device.device.Get(), heap, sizeof(fframe_data), std::format("CBV frame: back buffer {}", i).c_str()));
    }

    // Create light and material data SRV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      lights_data.emplace_back(fshader_resource_buffer::create(device.device.Get(), heap, sizeof(flight_properties) * MAX_LIGHTS, std::format("SRV lights: back buffer {}", i).c_str()));
    }
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      materials_data.emplace_back(fshader_resource_buffer::create(device.device.Get(), heap, sizeof(fmaterial_properties) * MAX_MATERIALS, std::format("SRV materials: back buffer {}", i).c_str()));
    }

    // Create first texture SRV (handles only)
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();
    // nonsense...
    default_texture->gpu_resource = ftexture_resource::create(device.device.Get(), heap, default_texture->gpu_resource.width, default_texture->gpu_resource.height,
      default_texture->gpu_resource.channels, default_texture->gpu_resource.element_size, DXGI_FORMAT_R8G8B8A8_UNORM, "default");
    
    // Set up graphics pipeline
    graphics_pipeline.reserve_parameters(root_parameter_type::num);
    graphics_pipeline.add_constant_parameter(root_parameter_type::object_data, 0, 0, sizeof(fobject_data), D3D12_SHADER_VISIBILITY_VERTEX);
    graphics_pipeline.add_constant_buffer_view_parameter(root_parameter_type::frame_data, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_shader_respurce_view_parameter(root_parameter_type::lights, 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_shader_respurce_view_parameter(root_parameter_type::materials, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_descriptor_table_parameter(root_parameter_type::textures, 2, 0, MAX_TEXTURES, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline.add_static_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
    graphics_pipeline.bind_pixel_shader(pixel_shader_asset.get()->render_state.blob);
    graphics_pipeline.bind_vertex_shader(vertex_shader_asset.get()->render_state.blob);
    graphics_pipeline.setup_formats(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT, { DXGI_FORMAT_R8G8B8A8_UNORM });
    graphics_pipeline.setup_input_layout({
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    });
    graphics_pipeline.init("Forward pass");
  }
  
  void fforward_pass::draw(ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    fdevice& device = fapplication::instance->device;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;

    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();
    
    //fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //fdx12::clear_render_target(device, command_list, rtv_descriptor_heap, back_buffer_index);
    //fdx12::clear_depth_stencil(command_list, dsv_descriptor_heap);
    //fdx12::set_render_targets(device, command_list, dsv_descriptor_heap, rtv_descriptor_heap, back_buffer_index);
    //fdx12::set_viewport(command_list, width, height);
    //fdx12::set_scissor(command_list, width, height);
        
    graphics_pipeline.bind_command_list(command_list.Get());

    command_list->SetDescriptorHeaps(1, heap->heap.GetAddressOf());

    const int back_buffer_index = context->back_buffer_index;
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
      {
        default_texture->gpu_resource.upload(device.device, command_list, default_texture->is_hdr ? reinterpret_cast<void*>(default_texture->data_hdr.data()) : default_texture->data_ldr.data());
      }

      // Upload other textures
      for(uint32_t i = 0; i < MAX_TEXTURES-1; i++)
      {
        if(i < num_textures_in_scene && scene_acceleration.a_textures[i] != default_texture)
        {
          atexture* texture = scene_acceleration.a_textures[i];
          if(!texture->gpu_resource.is_online)
          {
            texture->gpu_resource = ftexture_resource::create(device.device.Get(), heap, texture->gpu_resource.width, texture->gpu_resource.height,
              texture->gpu_resource.channels, texture->gpu_resource.element_size, DXGI_FORMAT_R8G8B8A8_UNORM, texture->get_display_name().c_str());
            
            texture->gpu_resource.upload(device.device, command_list, texture->is_hdr ? reinterpret_cast<void*>(texture->data_hdr.data()) : texture->data_ldr.data());
          }
        }
      }
    }
    
    // Update vertex and index buffers
    for(uint32_t i = 0; i < N; i++)
    {
      hstatic_mesh* mesh = scene_acceleration.h_meshes[i];
      fstatic_mesh_render_state& smrs = mesh->mesh_asset_ptr.get()->render_state;
      if(!smrs.is_resource_online)
      {
        fdx12::upload_vertex_buffer(device.device, command_list, smrs);
        fdx12::upload_index_buffer(device.device, command_list, smrs);
#if BUILD_DEBUG
        {
          std::string mesh_name = mesh->get_display_name();
          std::string asset_name = mesh->mesh_asset_ptr.get()->name;
          DX_SET_NAME(smrs.vertex_buffer, "Vertex buffer: asset {} hittable {}", mesh_name, asset_name)
          DX_SET_NAME(smrs.vertex_buffer_upload, "Index buffer: asset {} hittable {}", mesh_name, asset_name)
          DX_SET_NAME(smrs.index_buffer, "Vertex upload buffer: asset {} hittable {}", mesh_name, asset_name)
          DX_SET_NAME(smrs.index_buffer_upload, "Index upload buffer: asset {} hittable {}", mesh_name, asset_name)
        }
#endif
      }
    }
    
    // Draw
    for(uint32_t i = 0; i < N; i++)
    {
      const fstatic_mesh_render_state& smrs = context->scene->scene_acceleration.h_meshes[i]->mesh_asset_ptr.get()->render_state;

      command_list->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, sizeof(fobject_data)/4, &scene_acceleration.object_buffer[i], 0);
      command_list->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, frame_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::lights, lights_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::materials, materials_data[back_buffer_index].resource->GetGPUVirtualAddress());
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::textures, default_texture->gpu_resource.srv.gpu_handle);
      command_list->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list->DrawIndexedInstanced(static_cast<uint32_t>(smrs.vertex_list.size()), 1, 0, 0, 0);
    }
    
    //fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  }

  void fforward_pass::create_output_texture(bool cleanup)
  {
  }
}