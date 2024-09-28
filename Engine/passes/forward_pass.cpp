
#include "forward_pass.h"

#include <format>

#include "d3dx12/d3dx12.h"
#include "d3dx12/d3dx12_root_signature.h"

#include "core/application.h"
#include "core/exceptions.h"
#include "core/window.h"
#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "math/math.h"
#include "renderer/dx12_lib.h"
#include "renderer/render_state.h"
#include "renderer/aligned_structs.h"
#include "renderer/pipeline_state.h"
#include "renderer/scene_acceleration.h"

namespace engine
{
  using namespace DirectX;

  // Based on multiple training projects:
  // https://github.com/jpvanoosten/LearningDirectX12/tree/main/samples
  // https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop
  enum root_parameter_type
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

    if(!vertex_shader->blob)
    {
      LOG_ERROR("Failed to load vertex shader.");
      can_render = false;
      return;
    }
    if(!pixel_shader->blob)
    {
      LOG_ERROR("Failed to load index shader.");
      can_render = false;
      return;
    }

    // Build the main heap indexes
    const uint32_t main_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    cbv_frame_data_heap_index = 0;
    srv_lights_heap_index = cbv_frame_data_heap_index + window->back_buffer_count;
    srv_materials_heap_index = srv_lights_heap_index + window->back_buffer_count;
    srv_textures_heap_index = srv_materials_heap_index + window->back_buffer_count;
    if(MAX_TEXTURES + window->back_buffer_count * (MAX_MATERIALS + MAX_LIGHTS) > MAX_MAIN_DESCRIPTORS)
    {
      LOG_ERROR("Invalid main heap layout.");
      return;
    }
    
    // Create frame data CBV
    {
      // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables#c0
      for(uint32_t i = 0; i < window->back_buffer_count; i++)
      {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(window->main_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), cbv_frame_data_heap_index + i, main_descriptor_size);
        ComPtr<ID3D12Resource> resource;
        fdx12::create_const_buffer(device, handle, sizeof(fframe_data), resource);
        cbv_frame_resource.push_back(resource);
        
#if BUILD_DEBUG
        std::string name = std::format("CBV frame data upload resource: back buffer {}", i);
        resource->SetName(std::wstring(name.begin(), name.end()).c_str());
#endif
      }
    }

    // Create light and material data SRV
    {
      for(uint32_t i = 0; i < window->back_buffer_count; i++)
      {
        {
          CD3DX12_CPU_DESCRIPTOR_HANDLE handle(window->main_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), srv_lights_heap_index + i, main_descriptor_size);
          ComPtr<ID3D12Resource> resource;
          fdx12::create_shader_resource_buffer(device, handle, sizeof(flight_properties) * MAX_LIGHTS, resource);
          srv_lights_resource.push_back(resource);
          
#if BUILD_DEBUG
          std::string name = std::format("SRV lights data upload resource: back buffer {}", i);
          resource->SetName(std::wstring(name.begin(), name.end()).c_str());
#endif
        }
        {
          CD3DX12_CPU_DESCRIPTOR_HANDLE handle(window->main_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), srv_materials_heap_index + i, main_descriptor_size);
          ComPtr<ID3D12Resource> resource;
          fdx12::create_shader_resource_buffer(device, handle, sizeof(fmaterial_properties) * MAX_MATERIALS, resource);
          srv_materials_resource.push_back(resource);
          
#if BUILD_DEBUG
          std::string name = std::format("SRV materials data upload resource: back buffer {}", i);
          resource->SetName(std::wstring(name.begin(), name.end()).c_str());
#endif
        }
      }
    }
    
    // Root signature
    {
      // https://asawicki.info/news_1754_direct3d_12_long_way_to_access_data
      
      std::vector<CD3DX12_ROOT_PARAMETER1> root_parameters;
      const CD3DX12_ROOT_PARAMETER1 param = {};
      root_parameters.resize(root_parameter_type::num, param);
      {
        const uint8_t register_b0 = 0;
        const uint8_t register_b1 = 1;
        root_parameters[root_parameter_type::object_data].InitAsConstants(sizeof(fobject_data) / 4, register_b0, 0);
        root_parameters[root_parameter_type::frame_data].InitAsConstantBufferView(register_b1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);

        const uint8_t register_t0 = 0;
        const uint8_t register_t1 = 1;
        const uint8_t register_t2 = 2;
        root_parameters[root_parameter_type::lights].InitAsShaderResourceView(register_t0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
        root_parameters[root_parameter_type::materials].InitAsShaderResourceView(register_t1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
        CD3DX12_DESCRIPTOR_RANGE1 texture_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_TEXTURES, register_t2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, srv_textures_heap_index);
        root_parameters[root_parameter_type::textures].InitAsDescriptorTable(1, &texture_range, D3D12_SHADER_VISIBILITY_PIXEL);
      }

      std::vector<CD3DX12_STATIC_SAMPLER_DESC> static_sampers;
      CD3DX12_STATIC_SAMPLER_DESC sampler_desc(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
      static_sampers.push_back(sampler_desc);
      
      fdx12::create_root_signature(device, root_parameters, static_sampers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, root_signature);
#if BUILD_DEBUG
      root_signature->SetName(L"Root signature forward pass");
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
      IDxcBlob* vs_blob = vertex_shader->blob.Get();
      IDxcBlob* ps_blob = pixel_shader->blob.Get();
      fpipeline_state_stream pipeline_state_stream;
      pipeline_state_stream.root_signature = root_signature.Get();
      pipeline_state_stream.input_layout = { input_element_desc, _countof(input_element_desc) };
      pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      pipeline_state_stream.vertex_shader = CD3DX12_SHADER_BYTECODE(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize());
      pipeline_state_stream.pixel_shader = CD3DX12_SHADER_BYTECODE(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize());
      pipeline_state_stream.dsv_format = depth_buffer_format;
      pipeline_state_stream.rtv_formats = rtv_formats;
      fdx12::create_pipeline_state(device, pipeline_state_stream, pipeline_state);
#if BUILD_DEBUG
      pipeline_state->SetName(L"Pipeline state forward pass");
#endif
    }
  }
  
  void fforward_pass::draw(ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    command_list->SetGraphicsRootSignature(root_signature.Get());
    command_list->SetDescriptorHeaps(1, window->main_descriptor_heap.GetAddressOf());
    command_list->SetPipelineState(pipeline_state.Get());
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const int back_buffer_index = window->get_back_buffer_index();
    const uint32_t N = static_cast<uint32_t>(scene_acceleration->h_meshes.size());
    
    // Process object data
    for(uint32_t i = 0; i < N; i++)
    {
      fobject_data& object_data = scene_acceleration->object_buffer[i];
      const hstatic_mesh* sm = scene_acceleration->h_meshes[i];
      object_data.is_selected = selected_object == sm ? 1 : 0;
      object_data.object_id = fmath::uint32_to_colorf(sm->get_hash());
    }

    // Process frame data CBV
    {
      fframe_data frame_data;
      frame_data.camera_position = XMFLOAT4(scene->camera_config.location.e);
      frame_data.ambient_light = scene->ambient_light_color;
      frame_data.show_ambient = show_ambient;
      frame_data.show_diffuse = show_diffuse;
      frame_data.show_emissive = show_emissive;
      frame_data.show_normals = show_normals;
      frame_data.show_object_id = show_object_id;
      frame_data.show_specular = show_specular;
      fdx12::update_buffer(cbv_frame_resource[back_buffer_index], sizeof(fframe_data), &frame_data);
    }

    // Process light and material SRVs
    {
      const std::vector<flight_properties>& lights_buffer = scene_acceleration->lights_buffer;
      const std::vector<fmaterial_properties>& materials_buffer = scene_acceleration->materials_buffer;
    
      fdx12::update_buffer(srv_lights_resource[back_buffer_index], sizeof(flight_properties) * MAX_LIGHTS, lights_buffer.data());
      fdx12::update_buffer(srv_materials_resource[back_buffer_index], sizeof(fmaterial_properties) * MAX_MATERIALS, materials_buffer.data());
    }

    // Process texture SRVs
    {
      const uint32_t num_textures_in_scene = static_cast<uint32_t>(scene_acceleration->a_textures.size());
      const uint8_t descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      CD3DX12_CPU_DESCRIPTOR_HANDLE handle(window->main_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), srv_textures_heap_index, descriptor_size);
      
      // Upload default texture first
      atexture* default_texture = default_material_asset.get()->texture_asset_ptr.get();
      if(!default_texture->render_state.is_resource_online)
      {
        fdx12::upload_texture_buffer(device, command_list, window->main_descriptor_heap, handle, default_texture);
        handle.Offset(descriptor_size);
      }

      // Upload other textures, add descriptor if they are already uploaded
      for(uint32_t i = 0; i < MAX_TEXTURES-1; i++)
      {
        if(i < num_textures_in_scene && scene_acceleration->a_textures[i] != default_texture)
        {
          atexture* texture = scene_acceleration->a_textures[i];
          if(!texture->render_state.is_resource_online)
          {
            fdx12::upload_texture_buffer(device, command_list, window->main_descriptor_heap, handle, texture);
            handle.Offset(descriptor_size);
        
#if BUILD_DEBUG
            {
              std::string texture_name = texture->get_display_name();
              std::string name = std::format("Texture buffer: {}", texture_name);
              texture->render_state.texture_buffer->SetName(std::wstring(name.begin(), name.end()).c_str());
              name = std::format("Texture upload buffer: {}", texture_name);
              texture->render_state.texture_buffer_upload->SetName(std::wstring(name.begin(), name.end()).c_str());
            }
#endif
          }
        }
        else
        {
          // Describe and create a SRV for the texture.
          D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
          srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
          srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO hardcoded, read from texture asset
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
          srv_desc.Texture2D.MipLevels = 1;
          device->CreateShaderResourceView(default_texture->render_state.texture_buffer.Get(), &srv_desc, handle);
        
          handle.Offset(descriptor_size);
        }
      }
    }
    
    // Update vertex and index buffers
    for(uint32_t i = 0; i < N; i++)
    {
      hstatic_mesh* mesh = scene_acceleration->h_meshes[i];
      fstatic_mesh_render_state& smrs = mesh->mesh_asset_ptr.get()->render_state;
      if(!smrs.is_resource_online)
      {
        fdx12::upload_vertex_buffer(device, command_list, smrs);
        fdx12::upload_index_buffer(device, command_list, smrs);
#if BUILD_DEBUG
        {
          std::string mesh_name = mesh->get_display_name();
          std::string asset_name = mesh->mesh_asset_ptr.get()->file_name;
          std::string name = std::format("Vertex buffer: asset {} hittable {}", mesh_name, asset_name);
          smrs.vertex_buffer->SetName(std::wstring(name.begin(), name.end()).c_str());
          name = std::format("Index buffer: asset {} hittable {}", mesh_name, asset_name);
          smrs.index_buffer->SetName(std::wstring(name.begin(), name.end()).c_str());
          name = std::format("Vertex upload buffer: asset {} hittable {}", mesh_name, asset_name);
          smrs.vertex_buffer_upload->SetName(std::wstring(name.begin(), name.end()).c_str());
          name = std::format("Index upload buffer: asset {} hittable {}", mesh_name, asset_name);
          smrs.index_buffer_upload->SetName(std::wstring(name.begin(), name.end()).c_str());
        }
#endif
      }
    }
    
    // Draw
    for(uint32_t i = 0; i < N; i++)
    {
      const fstatic_mesh_render_state& smrs = scene_acceleration->h_meshes[i]->mesh_asset_ptr.get()->render_state;

      command_list->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, sizeof(fobject_data)/4, &scene_acceleration->object_buffer[i], 0);
      command_list->SetGraphicsRootConstantBufferView(root_parameter_type::frame_data, cbv_frame_resource[back_buffer_index]->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::lights, srv_lights_resource[back_buffer_index]->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::materials, srv_materials_resource[back_buffer_index]->GetGPUVirtualAddress());
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::textures, window->main_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
      command_list->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list->DrawIndexedInstanced(static_cast<uint32_t>(smrs.vertex_list.size()), 1, 0, 0, 0);
    }
  }

  void fforward_pass::create_output_texture(bool cleanup)
  {
    //if(cleanup)
    //{
    //  DX_RELEASE(output_rtv)
    //  DX_RELEASE(output_srv)
    //  DX_RELEASE(output_dsv)
    //  DX_RELEASE(output_texture)
    //  DX_RELEASE(output_depth)
    //}
    //
    //fdx12& dx = fdx12::instance();
    //DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //D3D11_BIND_FLAG bind_flag = static_cast<D3D11_BIND_FLAG>(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    //dx.create_texture(output_width, output_height, format, bind_flag, D3D11_USAGE_DEFAULT, output_texture);
    //dx.create_shader_resource_view(output_texture, format, D3D11_SRV_DIMENSION_TEXTURE2D, output_srv);
    //dx.create_render_target_view(output_texture, format, D3D11_RTV_DIMENSION_TEXTURE2D, output_rtv);
    //
    //dx.create_texture(output_width, output_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, output_depth);
    //dx.create_depth_stencil_view(output_depth, output_width, output_height, output_dsv);
  }
}