#include <d3d11_1.h>

#include "forward_pass.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "math/math.h"
#include "renderer/dx11_lib.h"
#include "renderer/render_state.h"
#include "renderer/aligned_structs.h"
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
      int32_t show_emissive; // 4    // TODO pack bits
      int32_t show_ambient; // 4
      int32_t show_specular; // 4
      int32_t show_diffuse; // 4
      int32_t show_normals; // 4
      int32_t show_object_id; // 4
      int32_t padding[2]; // 8
      flight_properties lights[MAX_LIGHTS]; // 80xN
      fmaterial_properties materials[MAX_MATERIALS]; // 80xN
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

    ALIGNED_STRUCT_END(fobject_data)
  }
  
  void fforward_pass::init()
  {
    auto dx = fdx11::instance();
    {
      D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
      {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
      };
      dx.create_input_layout(input_element_desc, ARRAYSIZE(input_element_desc), vertex_shader->shader_blob, input_layout);
    }
    dx.create_sampler_state(sampler_state);
    dx.create_constant_buffer(sizeof(fobject_data), object_constant_buffer);
    dx.create_constant_buffer(sizeof(fframe_data), frame_constant_buffer);
    dx.create_rasterizer_state(rasterizer_state);
    dx.create_depth_stencil_state(depth_stencil_state);
  }

  void fforward_pass::draw()
  {
    fdx11& dx = fdx11::instance();

    dx.device_context->ClearRenderTargetView(output_rtv.Get(), scene->clear_color);
    dx.device_context->ClearDepthStencilView(output_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    const D3D11_VIEWPORT viewport = {0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f};
    dx.device_context->RSSetViewports(1, &viewport);
    dx.device_context->RSSetState(rasterizer_state.Get());
    dx.device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);
    dx.device_context->OMSetRenderTargets(1, output_rtv.GetAddressOf(), output_dsv.Get());

    dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx.device_context->IASetInputLayout(input_layout.Get());

    dx.device_context->VSSetShader(vertex_shader->shader.Get(), nullptr, 0);
    dx.device_context->VSSetConstantBuffers(0, 1, object_constant_buffer.GetAddressOf());

    dx.device_context->PSSetShader(pixel_shader->shader.Get(), nullptr, 0);
    dx.device_context->PSSetConstantBuffers(0, 1, object_constant_buffer.GetAddressOf());
    dx.device_context->PSSetConstantBuffers(1, 1, frame_constant_buffer.GetAddressOf());

    dx.device_context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());

    // Update per-frame constant buffer
    {
      fframe_data pfd;
      scene_acceleration->get_lights_array(pfd.lights);
      scene_acceleration->get_materials_array(pfd.materials);
      pfd.camera_position = XMFLOAT4(scene->camera_config.location.e);
      pfd.ambient_light = scene->ambient_light_color;
      pfd.show_emissive = show_emissive;
      pfd.show_ambient = show_ambient;
      pfd.show_diffuse = show_diffuse;
      pfd.show_specular = show_specular;
      pfd.show_normals = show_normals;
      pfd.show_object_id = show_object_id;
      dx.update_constant_buffer<fframe_data>(&pfd, frame_constant_buffer);
    }

    // Draw the scene
    for(const hstatic_mesh* sm : scene_acceleration->meshes)
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

        fobject_data pod;
        XMStoreFloat4x4(&pod.model_world, world_matrix);
        XMStoreFloat4x4(&pod.inverse_transpose_model_world, inverse_transpose_model_world);
        XMStoreFloat4x4(&pod.model_world_view_projection, model_world_view_projection);
        if(const amaterial* material = sm->material_asset_ptr.get())
        {
          pod.material_id = scene_acceleration->material_map.at(material);
        }
        pod.is_selected = selected_object == sm ? 1 : 0;
        pod.object_id = fmath::uint32_to_colorf(sm->get_hash());
        dx.update_constant_buffer<fobject_data>(&pod, object_constant_buffer);
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

  void fforward_pass::create_output_texture(bool cleanup)
  {
    if(cleanup)
    {
      DX_RELEASE(output_rtv)
      DX_RELEASE(output_srv)
      DX_RELEASE(output_dsv)
      DX_RELEASE(output_texture)
      DX_RELEASE(output_depth)
    }

    fdx11& dx = fdx11::instance();
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    D3D11_BIND_FLAG bind_flag = static_cast<D3D11_BIND_FLAG>(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    dx.create_texture(output_width, output_height, format, bind_flag, D3D11_USAGE_DEFAULT, output_texture);
    dx.create_shader_resource_view(output_texture, format, D3D11_SRV_DIMENSION_TEXTURE2D, output_srv);
    dx.create_render_target_view(output_texture, format, D3D11_RTV_DIMENSION_TEXTURE2D, output_rtv);

    dx.create_texture(output_width, output_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, output_depth);
    dx.create_depth_stencil_view(output_depth, output_width, output_height, output_dsv);
  }
}