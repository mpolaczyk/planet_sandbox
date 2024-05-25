﻿#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_deferred_sync.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"

using namespace DirectX;

namespace engine
{
  OBJECT_DEFINE(rgpu_deferred_sync, rrenderer_base, GPU deferred sync)
  OBJECT_DEFINE_SPAWN(rgpu_deferred_sync)
  OBJECT_DEFINE_VISITOR(rgpu_deferred_sync)

  ALIGNED_STRUCT_BEGIN(fdeferred_object_data)
  {
    XMFLOAT4X4 model_world; // 64 Used to transform the vertex position from object space to world space
    XMFLOAT4X4 inverse_transpose_model_world; // 64 Used to transform the vertex normal from object space to world space
    XMFLOAT4X4 model_world_view_projection; // 64 Used to transform the vertex position from object space to projected clip space
    uint32_t material_id; // 4
    int32_t padding[3]; // 12
  };

  ALIGNED_STRUCT_END(fdeferred_object_data)

  bool rgpu_deferred_sync::can_render() const
  {
    if(!rrenderer_base::can_render())
    {
      return false;
    }
    if(gbuffer_vertex_shader_asset.get() == nullptr)
    {
      LOG_ERROR("Missing gbuffer vertex shader setup.");
      return false;
    }
    if(gbuffer_pixel_shader_asset.get() == nullptr)
    {
      LOG_ERROR("Missing gbuffer pixel shader setup.");
      return false;
    }
    return true;
  }

  void rgpu_deferred_sync::init()
  {
    auto dx = fdx11::instance();
    {
      D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
      {
        // Per-vertex
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
      };
      auto blob = gbuffer_vertex_shader_asset.get()->render_state.shader_blob;
      dx.create_input_layout(input_element_desc, ARRAYSIZE(input_element_desc), blob, input_layout);
    }
    dx.create_sampler_state(sampler_state);
    dx.create_constant_buffer(sizeof(fdeferred_object_data), object_constant_buffer);
    //dx.create_constant_buffer(sizeof(fframe_data), frame_constant_buffer);
    dx.create_rasterizer_state(rasterizer_state);
    dx.create_depth_stencil_state(depth_stencil_state);
  }

  void rgpu_deferred_sync::create_output_texture(bool cleanup)
  {
    rrenderer_base::create_output_texture(cleanup);

    if(cleanup)
    {
      for(int i = 0; i < ebuffer_type::count; i++)
      {
        DX_RELEASE(gbuffer_rtvs[i])
        DX_RELEASE(gbuffer_srvs[i])
        DX_RELEASE(gbuffer_textures[i])
      }
      DX_RELEASE(gbuffer_dsb)
      DX_RELEASE(gbuffer_dsv)
    }

    fdx11& dx = fdx11::instance();
    for(int i = 0; i < ebuffer_type::count; i++)
    {
      DXGI_FORMAT format = (i == ebuffer_type::material_id ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32G32B32A32_FLOAT);
      D3D11_BIND_FLAG bind_flag = static_cast<D3D11_BIND_FLAG>(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
      
      dx.create_texture(output_width, output_height, format, bind_flag, D3D11_USAGE_DEFAULT, gbuffer_textures[i]);
      dx.create_shader_resource_view(gbuffer_textures[i], format, D3D11_SRV_DIMENSION_TEXTURE2D, gbuffer_srvs[i]);
      dx.create_render_target_view(gbuffer_textures[i], format, D3D11_RTV_DIMENSION_TEXTURE2D, gbuffer_rtvs[i]);
    }
  }

  void rgpu_deferred_sync::render_frame_impl()
  {
    fdx11& dx = fdx11::instance();

    for(int i = 0; i < ebuffer_type::count; i++)
    {
      dx.device_context->ClearRenderTargetView(gbuffer_rtvs[i].Get(), scene->clear_color);
    }
    dx.device_context->ClearRenderTargetView(output_rtv.Get(), scene->clear_color);
    dx.device_context->ClearDepthStencilView(output_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    scene_acceleration.clean(MAX_LIGHTS, MAX_MATERIALS);
    scene_acceleration.build(scene->objects);
    if(!scene_acceleration.validate())
    {
      return;
    }

    const D3D11_VIEWPORT viewport = {0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f};
    dx.device_context->RSSetViewports(1, &viewport);
    dx.device_context->RSSetState(rasterizer_state.Get());
    dx.device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);
    ID3D11RenderTargetView* rtvs[ebuffer_type::count];
    for(int i = 0; i < ebuffer_type::count; i++)
    {
      rtvs[i] = gbuffer_rtvs[i].Get();  // FIX GetAddressOf?
    }
    dx.device_context->OMSetRenderTargets(ebuffer_type::count, rtvs, output_dsv.Get());

    dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx.device_context->IASetInputLayout(input_layout.Get());

    dx.device_context->VSSetShader(gbuffer_vertex_shader_asset.get()->render_state.shader.Get(), nullptr, 0);
    dx.device_context->VSSetConstantBuffers(0, 1, object_constant_buffer.GetAddressOf());

    dx.device_context->PSSetShader(gbuffer_pixel_shader_asset.get()->render_state.shader.Get(), nullptr, 0);
    //dx.device_context->PSSetConstantBuffers(0, 1, frame_constant_buffer.GetAddressOf());
    dx.device_context->PSSetConstantBuffers(1, 1, object_constant_buffer.GetAddressOf());

    dx.device_context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());

    // Update per-frame constant buffer
    {
      //fframe_data pfd;
      //scene_acceleration.get_lights_array(pfd.lights);
      //scene_acceleration.get_materials_array(pfd.materials);
      //pfd.camera_position = XMFLOAT4(scene->camera_config.location.e);
      //pfd.ambient_light = scene->ambient_light_color;
      //pfd.show_emissive = show_emissive;
      //pfd.show_ambient = show_ambient;
      //pfd.show_diffuse = show_diffuse;
      //pfd.show_specular = show_specular;
      //pfd.show_normals = show_normals;
      //pfd.show_object_id = show_object_id;
      //dx.update_constant_buffer<fframe_data>(&pfd, frame_constant_buffer);
    }

    // Draw the scene
    for(const hstatic_mesh* sm : scene_acceleration.meshes)
    {
      const astatic_mesh* sma = sm->mesh_asset_ptr.get();
      if(sma == nullptr) { continue; }
      const fstatic_mesh_render_state& smrs = sma->render_state;
      const amaterial* ma = sm->material_asset_ptr.get();
      if(ma == nullptr)
      {
        ma = default_material_asset.get();
      }
      const atexture* ta = ma->texture_asset_ptr.get();

      // Update per-object constant buffer
      {
        XMMATRIX translation_matrix = XMMatrixTranslation(sm->origin.x, sm->origin.y, sm->origin.z);
        XMMATRIX rotation_matrix = XMMatrixRotationX(XMConvertToRadians(sm->rotation.x))
          * XMMatrixRotationY(XMConvertToRadians(sm->rotation.y))
          * XMMatrixRotationZ(XMConvertToRadians(sm->rotation.z));
        XMMATRIX scale_matrix = XMMatrixScaling(sm->scale.x, sm->scale.y, sm->scale.z);
        XMMATRIX world_matrix = scale_matrix * rotation_matrix * translation_matrix;

        const XMMATRIX inverse_transpose_model_world = XMMatrixTranspose(XMMatrixInverse(nullptr, world_matrix));
        const XMMATRIX model_world_view_projection = XMMatrixMultiply(world_matrix, XMLoadFloat4x4(&scene->camera_config.view_projection));

        fdeferred_object_data pod;
        XMStoreFloat4x4(&pod.model_world, world_matrix);
        XMStoreFloat4x4(&pod.inverse_transpose_model_world, inverse_transpose_model_world);
        XMStoreFloat4x4(&pod.model_world_view_projection, model_world_view_projection);
        if(const amaterial* material = sm->material_asset_ptr.get())
        {
          pod.material_id = scene_acceleration.material_map[material];
        }
        dx.update_constant_buffer<fdeferred_object_data>(&pod, object_constant_buffer);
      }

      // Update texture
      if(ma->properties.use_texture)
      {
        if(ta == nullptr)
        {
          ta = default_material_asset.get()->texture_asset_ptr.get();
        }
        dx.device_context->PSSetShaderResources(0, 1, ta->render_state.texture_srv.GetAddressOf());
      }

      // Update mesh
      dx.device_context->IASetVertexBuffers(0, 1, smrs.vertex_buffer.GetAddressOf(), &smrs.stride, &smrs.offset);
      dx.device_context->IASetIndexBuffer(smrs.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
      static_assert(sizeof(fface_data_type) == sizeof(uint32_t));

      // Draw
      dx.device_context->DrawIndexed(smrs.num_indices, 0, 0);
    }
  }
}