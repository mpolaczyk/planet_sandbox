
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

  // Based on multiple training projects:
  // https://github.com/jpvanoosten/LearningDirectX12/tree/main/samples
  // https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop
  namespace
  {
    ALIGNED_STRUCT_BEGIN(fframe_data)
    {
      XMFLOAT4 camera_position; // 16
      XMFLOAT4 ambient_light; // 16
      int32_t show_emissive; // 4    // TODO pack bits
      int32_t show_ambient; // 4
      int32_t show_specular; // 4
      int32_t show_diffuse; // 4
      int32_t show_normals; // 4
      int32_t show_object_id; // 4
      int32_t padding[2]; // 8
    };

    ALIGNED_STRUCT_END(fframe_data)

    ALIGNED_STRUCT_BEGIN(fobject_data)
    {
      XMFLOAT4X4 model_world; // 64 Used to transform the vertex position from object space to world space
      XMFLOAT4X4 inverse_transpose_model_world; // 64 Used to transform the vertex normal from object space to world space
      XMFLOAT4X4 model_world_view_projection; // 64 Used to transform the vertex position from object space to projected clip space
      XMFLOAT4 object_id; // 16
      uint32_t material_id; // 4
      uint32_t is_selected; // 4
      int32_t padding[2]; // 8
    };
    static_assert(sizeof(fobject_data)/4 < 64); // "Root Constant size is greater than 64 DWORDs. Additional indirection may be added by the driver."
    ALIGNED_STRUCT_END(fobject_data)
  }

  enum root_parameter_type
  {
    constants = 0,
    cbv_frame_data,
    srv_lights_data,
    srv_materials_data,
    texture,
    num
  };
  
  void fforward_pass::init()
  {
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    // Create frame data CBV
    {
      // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables#c0
      for(uint32_t n = 0; n < window->back_buffer_count; n++)
      {
        ComPtr<ID3D12Resource> resource;
        uint8_t* mapping = nullptr;
        fdx12::create_const_buffer(device, window->main_descriptor_heap, sizeof(fframe_data), 0, &mapping, resource);
#if BUILD_DEBUG
        std::string name = std::format("CBV frame data upload resource: back buffer {}", n);
        resource->SetName(std::wstring(name.begin(), name.end()).c_str());
#endif
        cbv_frame_data.push_back(resource);
      }
    }

    // Create light and material data SRV
    {
      for(uint32_t n = 0; n < window->back_buffer_count; n++)
      {
        {
          ComPtr<ID3D12Resource> resource;
          uint8_t* mapping = nullptr;
          fdx12::create_shader_resource_buffer(device, window->main_descriptor_heap, sizeof(flight_properties) * MAX_LIGHTS, 0, &mapping, resource);
          srv_lights_data.push_back(resource);
#if BUILD_DEBUG
          std::string name = std::format("SRV lights data upload resource: back buffer {}", n);
          resource->SetName(std::wstring(name.begin(), name.end()).c_str());
#endif
        }
        {
          ComPtr<ID3D12Resource> resource;
          uint8_t* mapping = nullptr;
          fdx12::create_shader_resource_buffer(device, window->main_descriptor_heap, sizeof(fmaterial_properties) * MAX_MATERIALS, 0, &mapping, resource);
          srv_materials_data.push_back(resource);
#if BUILD_DEBUG
          std::string name = std::format("SRV materials data upload resource: back buffer {}", n);
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
        // TODO This is overkill as it all get copied for each draw call. Use descriptor table instead

        CD3DX12_DESCRIPTOR_RANGE1 texture_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        root_parameters[root_parameter_type::constants].InitAsConstants(sizeof(fobject_data) / 4, 0, 0);
        root_parameters[root_parameter_type::cbv_frame_data].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC); // TODO PIXEL ONLY
        root_parameters[root_parameter_type::srv_lights_data].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC); // TODO PIXEL ONLY
        root_parameters[root_parameter_type::srv_materials_data].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC); // TODO PIXEL ONLY
        root_parameters[root_parameter_type::texture].InitAsDescriptorTable(1, &texture_range, D3D12_SHADER_VISIBILITY_PIXEL);
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
      fpipeline_state_stream pipeline_state_stream;
      pipeline_state_stream.root_signature = root_signature.Get();
      pipeline_state_stream.input_layout = { input_element_desc, _countof(input_element_desc) };
      pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      pipeline_state_stream.vertex_shader = CD3DX12_SHADER_BYTECODE(vertex_shader->blob.Get());
      pipeline_state_stream.pixel_shader = CD3DX12_SHADER_BYTECODE(pixel_shader->blob.Get());
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
    
    command_list->SetPipelineState(pipeline_state.Get());
    command_list->SetGraphicsRootSignature(root_signature.Get());
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const int back_buffer_index = window->get_back_buffer_index();
    
    // Calculate per-frame root descriptor arguments
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
      fdx12::update_buffer(cbv_frame_data[back_buffer_index], sizeof(fframe_data), &frame_data);
      fdx12::update_buffer(srv_lights_data[back_buffer_index], sizeof(flight_properties) * MAX_LIGHTS, &scene_acceleration->lights);
      fdx12::update_buffer(srv_materials_data[back_buffer_index], sizeof(fmaterial_properties) * MAX_MATERIALS, &scene_acceleration->materials);
    }
    
    // Continuous buffers
    const uint32_t buffers_num = scene_acceleration->meshes.size();
    const std::vector<hstatic_mesh*>& buffer_meshes = scene_acceleration->meshes;
    const std::vector<astatic_mesh*>& buffer_assets = scene_acceleration->assets;
    std::vector<fobject_data> buffer_object_data;
    buffer_object_data.resize(buffers_num, fobject_data());

    // TODO - upload all used textures here it happens per draw call now
    
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
          const hstatic_mesh* sm = buffer_meshes[i];
          std::string mesh_name = sm->get_display_name();
          std::string asset_name = buffer_assets[i]->file_name;
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
      object_data.is_selected = selected_object == sm ? 1 : 0;
      object_data.object_id = fmath::uint32_to_colorf(sm->get_hash());
    }

    // Draw
    for(uint32_t i = 0; i < buffers_num; i++)
    {
      // Update texture
      // TODO move it to the continuous buffer, let it be generated by the scene accelerator
      {
        //hstatic_mesh* sm = buffer_meshes[i];
        //amaterial* ma = sm->material_asset_ptr.get();
        //ma = (ma == nullptr) ? default_material_asset.get() : ma;
        //atexture* ta = ma->texture_asset_ptr.get();
        //ta = (ta == nullptr && ma->properties.use_texture) ? default_material_asset.get()->texture_asset_ptr.get() : ta;

        atexture* ta = default_material_asset.get()->texture_asset_ptr.get();
        
        if(!ta->render_state.is_resource_online)
        {
          fdx12::upload_texture_buffer(device, command_list, window->main_descriptor_heap, ta);
#if BUILD_DEBUG
          {
            std::string texture_name = ta->get_display_name();
            std::string name = std::format("Texture buffer: {}", texture_name);
            ta->render_state.texture_buffer->SetName(std::wstring(name.begin(), name.end()).c_str());
            name = std::format("Texture upload buffer: {}", texture_name);
            ta->render_state.texture_buffer_upload->SetName(std::wstring(name.begin(), name.end()).c_str());
          }
#endif
        }
      }
        // draw calls
      const fstatic_mesh_render_state& smrs = buffer_assets[i]->render_state;

      command_list->SetGraphicsRoot32BitConstants(root_parameter_type::constants, sizeof(fobject_data)/4, &buffer_object_data[i], 0);
      command_list->SetGraphicsRootConstantBufferView(root_parameter_type::cbv_frame_data, cbv_frame_data[back_buffer_index]->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::srv_lights_data, srv_lights_data[back_buffer_index]->GetGPUVirtualAddress());
      command_list->SetGraphicsRootShaderResourceView(root_parameter_type::srv_materials_data, srv_materials_data[back_buffer_index]->GetGPUVirtualAddress());
      command_list->SetGraphicsRootDescriptorTable(root_parameter_type::texture, window->main_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
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