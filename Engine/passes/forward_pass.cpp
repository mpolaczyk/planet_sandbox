
#include "forward_pass.h"

#include <format>

#include "d3dx12/d3dx12.h"
#include "d3dx12/d3dx12_root_signature.h"

#include "core/application.h"
#include "core/exceptions.h"
#include "core/window.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "math/math.h"
#include "renderer/dx12_lib.h"
#include "renderer/render_state.h"
#include "renderer/aligned_structs.h"
#include "renderer/pipeline_state.h"
#include "renderer/render_context.h"
#include "renderer/scene_acceleration.h"

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
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    if(!vertex_shader_blob)
    {
      LOG_ERROR("Failed to load vertex shader.");
      can_draw = false;
      return;
    }
    if(!pixel_shader_blob)
    {
      LOG_ERROR("Failed to load index shader.");
      can_draw = false;
      return;
    }
    if(MAX_TEXTURES + context->window->back_buffer_count * (MAX_MATERIALS + MAX_LIGHTS) > MAX_MAIN_DESCRIPTORS)
    {
      LOG_ERROR("Invalid main heap layout.");
      return;
    }

    uint32_t back_buffer_count = context->window->back_buffer_count;
    fdescriptor_heap& heap = context->window->main_descriptor_heap;
    
    // Create frame data CBV
    // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables#c0
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fdescriptor* d = heap.push(sizeof(fframe_data));
      frame_data_heap_index.push_back(d->index);
      fdx12::create_const_buffer(fapplication::instance->device, d->cpu_handle, d->resource_size, d->resource);
#if BUILD_DEBUG
      DX_SET_NAME(d->resource, "CBV frame: back buffer {}", i)
#endif
    }

    // Create light and material data SRV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fdescriptor* d = heap.push(sizeof(flight_properties) * MAX_LIGHTS);
      lights_data_heap_index.push_back(d->index);
      fdx12::create_shader_resource_buffer(fapplication::instance->device, d->cpu_handle, d->resource_size, d->resource);
#if BUILD_DEBUG
      DX_SET_NAME(d->resource, "SRV lights: back buffer {}", i)
#endif
    }
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fdescriptor* d = heap.push(sizeof(fmaterial_properties) * MAX_MATERIALS);
      materials_data_heap_index.push_back(d->index);
      fdx12::create_shader_resource_buffer(fapplication::instance->device, d->cpu_handle, d->resource_size, d->resource);
#if BUILD_DEBUG
      DX_SET_NAME(d->resource, "SRV materials: back buffer {}", i)
#endif
    }

    // Create first texture SRV (handles only)
    default_texture_heap_index = heap.push(0)->index;
    
    // Set up graphics pipeline
    {
      graphics_pipeline.reserve_parameters(root_parameter_type::num);
      graphics_pipeline.add_constant_parameter(root_parameter_type::object_data, 0, 0, sizeof(fobject_data), D3D12_SHADER_VISIBILITY_VERTEX);
      graphics_pipeline.add_constant_buffer_view_parameter(root_parameter_type::frame_data, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
      graphics_pipeline.add_shader_respurce_view_parameter(root_parameter_type::lights, 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
      graphics_pipeline.add_shader_respurce_view_parameter(root_parameter_type::materials, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
      graphics_pipeline.add_descriptor_table_parameter(root_parameter_type::textures, 2, 0, MAX_TEXTURES, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);

      graphics_pipeline.add_static_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);

      graphics_pipeline.bind_pixel_shader(pixel_shader_blob);
      graphics_pipeline.bind_vertex_shader(vertex_shader_blob);
      
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
  }
  
  void fforward_pass::draw(ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    ComPtr<ID3D12Device2> device = fapplication::instance->device;
    fdescriptor_heap& heap = context->window->main_descriptor_heap;

    graphics_pipeline.bind_command_list(command_list.Get());

    command_list->SetDescriptorHeaps(1, heap.heap.GetAddressOf());

    const int back_buffer_index = context->window->get_back_buffer_index();
    const uint32_t N = static_cast<uint32_t>(context->scene_acceleration->h_meshes.size());
    
    // Process object data
    for(uint32_t i = 0; i < N; i++)
    {
      fobject_data& object_data = context->scene_acceleration->object_buffer[i];
      const hstatic_mesh* sm = context->scene_acceleration->h_meshes[i];
      object_data.is_selected = context->selected_object == sm ? 1 : 0;
      object_data.object_id = fmath::uint32_to_colorf(sm->get_hash());
    }

    // Process frame data CBV
    {
      fframe_data frame_data;
      frame_data.camera_position = XMFLOAT4(context->scene->camera_config.location.e);
      frame_data.ambient_light = context->scene->ambient_light_color;
      frame_data.show_ambient = show_ambient;
      frame_data.show_diffuse = show_diffuse;
      frame_data.show_emissive = show_emissive;
      frame_data.show_normals = show_normals;
      frame_data.show_object_id = show_object_id;
      frame_data.show_specular = show_specular;
      fdescriptor* frame_entry = heap.get(frame_data_heap_index[back_buffer_index]);
      fdx12::update_buffer(frame_entry->resource, frame_entry->resource_size, &frame_data);
    }

    // Process light and material SRVs
    {
      fdescriptor* lights_d = heap.get(lights_data_heap_index[back_buffer_index]);
      fdescriptor* materials_d = heap.get(materials_data_heap_index[back_buffer_index]);
      fdx12::update_buffer(lights_d->resource, lights_d->resource_size, context->scene_acceleration->lights_buffer.data());
      fdx12::update_buffer(materials_d->resource, materials_d->resource_size, context->scene_acceleration->materials_buffer.data());
    }

    // Process texture SRVs
    {
      const uint32_t num_textures_in_scene = static_cast<uint32_t>(context->scene_acceleration->a_textures.size());

      // Upload default texture first
      atexture* default_texture = context->default_material_asset.get()->texture_asset_ptr.get();
      if(!default_texture->render_state.is_resource_online)
      {
        fdx12::upload_texture_buffer(device, command_list, heap.get(default_texture_heap_index)->cpu_handle, default_texture);
#if BUILD_DEBUG
        DX_SET_NAME(default_texture->render_state.texture_buffer, "Default texture buffer: {}", default_texture->get_display_name())
        DX_SET_NAME(default_texture->render_state.texture_buffer_upload, "Default texture upload buffer: {}", default_texture->get_display_name())
#endif
      }

      // Upload other textures
      for(uint32_t i = 0; i < MAX_TEXTURES-1; i++)
      {
        if(i < num_textures_in_scene && context->scene_acceleration->a_textures[i] != default_texture)
        {
          atexture* texture = context->scene_acceleration->a_textures[i];
          if(!texture->render_state.is_resource_online)
          {
            fdx12::upload_texture_buffer(device, command_list, heap.push(0)->cpu_handle, texture);
#if BUILD_DEBUG
            DX_SET_NAME(texture->render_state.texture_buffer, "Texture buffer: {}", texture->get_display_name())
            DX_SET_NAME(texture->render_state.texture_buffer_upload, "Texture upload buffer: {}", texture->get_display_name())
#endif
          }
        }
      }
    }
    
    // Update vertex and index buffers
    for(uint32_t i = 0; i < N; i++)
    {
      hstatic_mesh* mesh = context->scene_acceleration->h_meshes[i];
      fstatic_mesh_render_state& smrs = mesh->mesh_asset_ptr.get()->render_state;
      if(!smrs.is_resource_online)
      {
        fdx12::upload_vertex_buffer(device, command_list, smrs);
        fdx12::upload_index_buffer(device, command_list, smrs);
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
      const fstatic_mesh_render_state& smrs = context->scene_acceleration->h_meshes[i]->mesh_asset_ptr.get()->render_state;

      command_list->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, sizeof(fobject_data)/4, &context->scene_acceleration->object_buffer[i], 0);
      command_list->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, heap.get(frame_data_heap_index[back_buffer_index])->resource->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::lights, heap.get(lights_data_heap_index[back_buffer_index])->resource->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::materials, heap.get(materials_data_heap_index[back_buffer_index])->resource->GetGPUVirtualAddress());
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::textures, heap.get(default_texture_heap_index)->gpu_handle);
      command_list->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list->DrawIndexedInstanced(static_cast<uint32_t>(smrs.vertex_list.size()), 1, 0, 0, 0);
    }
  }

  void fforward_pass::create_output_texture(bool cleanup)
  {
  }
}