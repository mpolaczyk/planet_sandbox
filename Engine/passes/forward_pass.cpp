
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

    const uint32_t descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ID3D12DescriptorHeap* heap = context->window->main_descriptor_heap.Get();
    uint32_t back_buffer_count = context->window->back_buffer_count;
    int32_t heap_index = 0;
    
    // Create frame data CBV
    {
      // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables#c0
      for(uint32_t i = 0; i < back_buffer_count; i++)
      {
        fresource resource;
        resource.init(heap, heap_index++, descriptor_size);
        fdx12::create_const_buffer(device, resource.cpu_handle, sizeof(fframe_data), resource.resource);
        cbv_frame_resource.push_back(resource);
#if BUILD_DEBUG
        DX_SET_NAME(resource.resource, "CBV frame data upload resource: back buffer {}", i)
#endif
      }
    }

    // Create light and material data SRV
    {
      for(uint32_t i = 0; i < back_buffer_count; i++)
      {
        {
          fresource resource;
          resource.init(heap, heap_index++, descriptor_size);
          fdx12::create_shader_resource_buffer(device, resource.cpu_handle, sizeof(flight_properties) * MAX_LIGHTS, resource.resource);
          srv_lights_resource.push_back(resource);
#if BUILD_DEBUG
          DX_SET_NAME(resource.resource, "SRV lights data upload resource: back buffer {}", i)
#endif
        }
        {
          fresource resource;
          resource.init(heap, heap_index++, descriptor_size);
          fdx12::create_shader_resource_buffer(device, resource.cpu_handle, sizeof(fmaterial_properties) * MAX_MATERIALS, resource.resource);
          srv_materials_resource.push_back(resource);
#if BUILD_DEBUG
          DX_SET_NAME(resource.resource, "SRV materials data upload resource: back buffer {}", i)
#endif
        }
      }
    }

    // Create first texture SRV (handles only)
    srv_first_texture.init(heap, heap_index++, descriptor_size);
    
    // Root signature
    {
      // https://asawicki.info/news_1754_direct3d_12_long_way_to_access_data
      CD3DX12_DESCRIPTOR_RANGE1 frame_range(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
      CD3DX12_DESCRIPTOR_RANGE1 lights_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
      CD3DX12_DESCRIPTOR_RANGE1 materials_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
      CD3DX12_DESCRIPTOR_RANGE1 texture_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_TEXTURES, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

      std::vector<CD3DX12_ROOT_PARAMETER1> root_parameters(root_parameter_type::num);
      root_parameters[root_parameter_type::object_data].InitAsConstants(sizeof(fobject_data) / 4, 0, 0);
      root_parameters[root_parameter_type::frame_data].InitAsDescriptorTable(1, &frame_range, D3D12_SHADER_VISIBILITY_PIXEL);
      root_parameters[root_parameter_type::lights].InitAsDescriptorTable(1, &lights_range, D3D12_SHADER_VISIBILITY_PIXEL);
      root_parameters[root_parameter_type::materials].InitAsDescriptorTable(1, &materials_range, D3D12_SHADER_VISIBILITY_PIXEL);
      root_parameters[root_parameter_type::textures].InitAsDescriptorTable(1, &texture_range, D3D12_SHADER_VISIBILITY_PIXEL);
      
      std::vector<CD3DX12_STATIC_SAMPLER_DESC> static_sampers;
      CD3DX12_STATIC_SAMPLER_DESC sampler_desc(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
      static_sampers.push_back(sampler_desc);
      
      fdx12::create_root_signature(device, root_parameters, static_sampers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, graphics_pipeline.signature);
#if BUILD_DEBUG
      DX_SET_NAME(graphics_pipeline.signature, "Root signature forward pass")
#endif
    }

    // Pipeline state
    {
      D3D12_INPUT_ELEMENT_DESC input_element_desc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      };

      DXGI_FORMAT back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
      DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_D32_FLOAT;

      D3D12_RT_FORMAT_ARRAY rtv_formats = {};
      rtv_formats.NumRenderTargets = 1;
      rtv_formats.RTFormats[0] = back_buffer_format;

      // Pipeline state object
      IDxcBlob* vs_blob = vertex_shader_blob.Get();
      IDxcBlob* ps_blob = pixel_shader_blob.Get();
      fpipeline_state_stream pipeline_state_stream;
      pipeline_state_stream.root_signature = graphics_pipeline.signature.Get();
      pipeline_state_stream.input_layout = { input_element_desc, _countof(input_element_desc) };
      pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      pipeline_state_stream.vertex_shader = CD3DX12_SHADER_BYTECODE(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize());
      pipeline_state_stream.pixel_shader = CD3DX12_SHADER_BYTECODE(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize());
      pipeline_state_stream.dsv_format = depth_buffer_format;
      pipeline_state_stream.rtv_formats = rtv_formats;
      fdx12::create_pipeline_state(device, pipeline_state_stream, graphics_pipeline.state);
#if BUILD_DEBUG
      DX_SET_NAME(graphics_pipeline.state, "Pipeline state forward pass")
#endif
    }
  }
  
  void fforward_pass::draw(ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    command_list->SetGraphicsRootSignature(graphics_pipeline.signature.Get());
    command_list->SetDescriptorHeaps(1, context->window->main_descriptor_heap.GetAddressOf());
    command_list->SetPipelineState(graphics_pipeline.state.Get());
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
      fdx12::update_buffer(cbv_frame_resource[back_buffer_index].resource, sizeof(fframe_data), &frame_data);
    }

    // Process light and material SRVs
    {
      const std::vector<flight_properties>& lights_buffer = context->scene_acceleration->lights_buffer;
      const std::vector<fmaterial_properties>& materials_buffer = context->scene_acceleration->materials_buffer;
    
      fdx12::update_buffer(srv_lights_resource[back_buffer_index].resource, sizeof(flight_properties) * MAX_LIGHTS, lights_buffer.data());
      fdx12::update_buffer(srv_materials_resource[back_buffer_index].resource, sizeof(fmaterial_properties) * MAX_MATERIALS, materials_buffer.data());
    }

    // Process texture SRVs
    {
      ID3D12DescriptorHeap* heap = context->window->main_descriptor_heap.Get();
      const uint32_t num_textures_in_scene = static_cast<uint32_t>(context->scene_acceleration->a_textures.size());
      const uint32_t descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle = srv_first_texture.cpu_handle;
      
      // Upload default texture first
      atexture* default_texture = context->default_material_asset.get()->texture_asset_ptr.get();
      if(!default_texture->render_state.is_resource_online)
      {
        fdx12::upload_texture_buffer(device, command_list, heap, cpu_handle, default_texture);
        cpu_handle.Offset(descriptor_size);
      }

      // Upload other textures, add descriptor if they are already uploaded
      for(uint32_t i = 0; i < MAX_TEXTURES-1; i++)
      {
        if(i < num_textures_in_scene && context->scene_acceleration->a_textures[i] != default_texture)
        {
          atexture* texture = context->scene_acceleration->a_textures[i];
          if(!texture->render_state.is_resource_online)
          {
            fdx12::upload_texture_buffer(device, command_list, heap, cpu_handle, texture);
            cpu_handle.Offset(descriptor_size);
#if BUILD_DEBUG
            DX_SET_NAME(texture->render_state.texture_buffer, "Texture buffer: {}", texture->get_display_name())
            DX_SET_NAME(texture->render_state.texture_buffer_upload, "Texture upload buffer: {}", texture->get_display_name())
#endif
          }
        }
        //else
        //{
        //  // Describe and create a SRV for the texture.
        //  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        //  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        //  srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO hardcoded, read from texture asset
        //  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        //  srv_desc.Texture2D.MipLevels = 1;
        //  device->CreateShaderResourceView(default_texture->render_state.texture_buffer.Get(), &srv_desc, cpu_handle);
        //
        //  cpu_handle.Offset(descriptor_size);
        //}
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
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::frame_data, cbv_frame_resource[back_buffer_index].gpu_handle);
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::lights, srv_lights_resource[back_buffer_index].gpu_handle);
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::materials, srv_materials_resource[back_buffer_index].gpu_handle);
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::textures, srv_first_texture.gpu_handle);
      command_list->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list->DrawIndexedInstanced(static_cast<uint32_t>(smrs.vertex_list.size()), 1, 0, 0, 0);
    }
  }

  void fforward_pass::create_output_texture(bool cleanup)
  {
  }
}