
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
#include "renderer/scene_acceleration.h"

namespace engine
{
  using namespace DirectX;
  
  namespace
  {
    ALIGNED_STRUCT_BEGIN(fframe_data)
    {
      XMFLOAT4 camera_position; // 16
      XMFLOAT4 ambient_light; // 16
      //int32_t show_emissive; // 4    // TODO pack bits
      //int32_t show_ambient; // 4
      //int32_t show_specular; // 4
      //int32_t show_diffuse; // 4
      //int32_t show_normals; // 4
      //int32_t show_object_id; // 4
      //int32_t padding[2]; // 8
      flight_properties lights[MAX_LIGHTS]; // 80xN
      fmaterial_properties materials[MAX_MATERIALS]; // 80xN
    };

    ALIGNED_STRUCT_END(fframe_data)

    ALIGNED_STRUCT_BEGIN(fobject_data)
    {
      XMFLOAT4X4 model_world; // 64 Used to transform the vertex position from object space to world space
      XMFLOAT4X4 inverse_transpose_model_world; // 64 Used to transform the vertex normal from object space to world space
      XMFLOAT4X4 model_world_view_projection; // 64 Used to transform the vertex position from object space to projected clip space
      //XMFLOAT4 object_id; // 16
      uint32_t material_id; // 4
      //uint32_t is_selected; // 4
      int32_t padding[3]; // 12
    };
    static_assert(sizeof(fobject_data)/4 < 64); // "Root Constant size is greater than 64 DWORDs. Additional indirection may be added by the driver."
    ALIGNED_STRUCT_END(fobject_data)
  }
  
  void fforward_pass::init()
  {
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    // Constant buffer
    {
      // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables#c0
      for(uint32_t n = 0; n < window->back_buffer_count; n++)
      {
        ComPtr<ID3D12Resource> resource;
        uint8_t* mapping = nullptr;

        fdx12::create_const_buffer(device, window->main_descriptor_heap, sizeof(fframe_data), 0, &mapping, resource);

#if BUILD_DEBUG
        std::string name = std::format("Constant buffer upload resource: back buffer {}", n);
        resource->SetName(std::wstring(name.begin(), name.end()).c_str());
#endif
        cbv_mapping.push_back(mapping);
        cbv.push_back(resource);
      }
    }
    
    // Root signature
    {
      // https://asawicki.info/news_1754_direct3d_12_long_way_to_access_data
      
      std::vector<CD3DX12_ROOT_PARAMETER1> root_parameters;
      {
        CD3DX12_ROOT_PARAMETER1 param;
        param.InitAsConstants(sizeof(fobject_data) / 4, 0);
        root_parameters.push_back(param);
        
        param.InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC);
        root_parameters.push_back(param);
      }
      fdx12::create_root_signature(device, root_parameters, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, root_signature);
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
    
      D3D12_RT_FORMAT_ARRAY rtv_formats = {};
      rtv_formats.NumRenderTargets = 1;
      rtv_formats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

      // Pipeline state object
      fpipeline_state_stream pipeline_state_stream;
      pipeline_state_stream.root_signature = root_signature.Get();
      pipeline_state_stream.input_layout = { input_element_desc, _countof(input_element_desc) };
      pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      pipeline_state_stream.vertex_shader = CD3DX12_SHADER_BYTECODE(vertex_shader->blob.Get());
      pipeline_state_stream.pixel_shader = CD3DX12_SHADER_BYTECODE(pixel_shader->blob.Get());
      pipeline_state_stream.dsv_format = DXGI_FORMAT_D32_FLOAT;
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
    
    command_list->SetPipelineState(pipeline_state.Get());
    command_list->SetGraphicsRootSignature(root_signature.Get());
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // TODO Duplicate setting? Pipeline state already has one
    
    // Root descriptor argument
    {
      const int back_buffer_index = window->get_back_buffer_index();

      fframe_data frame_data;
      frame_data.camera_position = XMFLOAT4(scene->camera_config.location.e);
      frame_data.ambient_light = scene->ambient_light_color;
      memcpy(&frame_data.lights, &scene_acceleration->lights, MAX_LIGHTS * sizeof(flight_properties));
      memcpy(&frame_data.materials, &scene_acceleration->materials, MAX_MATERIALS * sizeof(fmaterial_properties));
      memcpy(cbv_mapping[back_buffer_index], &frame_data, sizeof(fframe_data));

      command_list->SetGraphicsRootConstantBufferView(1, cbv[back_buffer_index]->GetGPUVirtualAddress());
    }
    
    // Continuous buffers
    const uint32_t buffers_num = scene_acceleration->meshes.size();
    std::vector<hstatic_mesh*>& buffer_meshes = scene_acceleration->meshes;
    std::vector<astatic_mesh*>& buffer_assets = scene_acceleration->assets;
    std::vector<fobject_data> buffer_object_data;
    buffer_object_data.resize(buffers_num, fobject_data());

    // Calculate per-object root constant arguments
    for(uint32_t i = 0; i < buffers_num; i++)
    {
      const hstatic_mesh* sm = buffer_meshes[i];
      
      const XMMATRIX translation_matrix = XMMatrixTranslation(sm->origin.x, sm->origin.y, sm->origin.z);
      const XMMATRIX rotation_matrix =
          XMMatrixRotationX(XMConvertToRadians(sm->rotation.x))
        * XMMatrixRotationY(XMConvertToRadians(sm->rotation.y))
        * XMMatrixRotationZ(XMConvertToRadians(sm->rotation.z));
      const XMMATRIX scale_matrix = XMMatrixScaling(sm->scale.x, sm->scale.y, sm->scale.z);
      const XMMATRIX world_matrix = scale_matrix * rotation_matrix * translation_matrix;
      const XMMATRIX inverse_transpose_model_world = XMMatrixTranspose(XMMatrixInverse(nullptr, world_matrix));
      const XMMATRIX model_world_view_projection = XMMatrixMultiply(world_matrix, XMLoadFloat4x4(&scene->camera_config.view_projection));
      
      fobject_data& object_data = buffer_object_data[i];
      XMStoreFloat4x4(&object_data.model_world, world_matrix);
      XMStoreFloat4x4(&object_data.inverse_transpose_model_world, inverse_transpose_model_world);
      XMStoreFloat4x4(&object_data.model_world_view_projection, model_world_view_projection);
      object_data.material_id = 0;
      if(const amaterial* material = sm->material_asset_ptr.get())
      {
         object_data.material_id = scene_acceleration->material_map.at(material);
      }
    }

    // Update vertex and index buffers
    for(uint32_t i = 0; i < buffers_num; i++)
    {
      fstatic_mesh_render_state& smrs = buffer_assets[i]->render_state;
      if(!smrs.is_resource_online)
      {
        fdx12::upload_vertex_buffer(device, command_list, smrs);
        fdx12::upload_index_buffer(device, command_list, smrs);
#if BUILD_DEBUG
        {
          hstatic_mesh* sm = buffer_meshes[i];
          std::string mesh_name = sm->get_display_name();
          std::string asset_name = buffer_assets[i]->file_name;
          std::string vertex_name = std::format("Vertex buffer: asset {} hittable {}", mesh_name, asset_name);
          smrs.vertex_buffer->SetName(std::wstring(vertex_name.begin(), vertex_name.end()).c_str());
          std::string index_name = std::format("Index buffer: asset {} hittable {}", mesh_name, asset_name);
          smrs.index_buffer->SetName(std::wstring(index_name.begin(), index_name.end()).c_str());
        }
#endif
      }
    }

    // Draw
    for(uint32_t i = 0; i < buffers_num; i++)
    {
      command_list->SetGraphicsRoot32BitConstants(0, sizeof(fobject_data)/4, &buffer_object_data[i], 0);
      const fstatic_mesh_render_state& smrs = buffer_assets[i]->render_state;
      command_list->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list->DrawIndexedInstanced(smrs.vertex_list.size(), 1, 0, 0, 0);
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