#include "gbuffer_pass.h"

#include "core/application.h"
#include "core/core.h"
#include "core/window.h"
#include "engine/log.h"
#include "renderer/render_context.h"
#include "hittables/scene.h"
#include "renderer/scene_acceleration.h"

namespace engine
{
  using namespace DirectX;
  
  void fgbuffer_pass::init()
  {
    fpass_base::init();

    // Set up graphics pipeline
    {
      graphics_pipeline.reserve_parameters(1);
      graphics_pipeline.add_constant_parameter(0, 0, 0, static_cast<uint32_t>(sizeof(fobject_data)), D3D12_SHADER_VISIBILITY_VERTEX);
      graphics_pipeline.add_static_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
      graphics_pipeline.bind_pixel_shader(pixel_shader_asset.get()->resource.blob);
      graphics_pipeline.bind_vertex_shader(vertex_shader_asset.get()->resource.blob);
      graphics_pipeline.setup_formats(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT,
        {
          DXGI_FORMAT_R32G32B32A32_FLOAT,
          DXGI_FORMAT_R32G32B32A32_FLOAT,
          DXGI_FORMAT_R32G32B32A32_FLOAT,
          DXGI_FORMAT_R32G32B32A32_FLOAT,
          DXGI_FORMAT_R8_UINT,
          DXGI_FORMAT_R8_UINT
        });
      graphics_pipeline.setup_input_layout({
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      });
      graphics_pipeline.init("GBuffer pass");
    }
  }

  void fgbuffer_pass::draw(std::shared_ptr<fgraphics_command_list> command_list)
  {
    //fdescriptor_heap* heap = context->main_descriptor_heap;

    //graphics_pipeline.bind_command_list(command_list.Get());

    //command_list->SetDescriptorHeaps(1, heap->heap.GetAddressOf());

    //const int back_buffer_index = context->back_buffer_index;
    //const uint32_t N = static_cast<uint32_t>(context->scene.scene_acceleration->h_meshes.size());

    
//    for(int i = 0; i < egbuffer_type::count; i++)
//    {
//      dx.device_context->ClearRenderTargetView(output_rtv[i].Get(), scene->clear_color);
//    }
//    dx.device_context->ClearDepthStencilView(output_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
//    
//    const D3D11_VIEWPORT viewport = {0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f};
//    dx.device_context->RSSetViewports(1, &viewport);
//    dx.device_context->RSSetState(rasterizer_state.Get());
//    dx.device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);
//    ID3D11RenderTargetView* const* rtvsp[egbuffer_type::count];
//    for(int i = 0; i < egbuffer_type::count; i++)
//    {
//      rtvsp[i] = output_rtv[i].GetAddressOf();
//    }
//    dx.device_context->OMSetRenderTargets(egbuffer_type::count, rtvsp[0], output_dsv.Get());
//
//    dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    dx.device_context->IASetInputLayout(input_layout.Get());
//
//    dx.device_context->VSSetShader(vertex_shader->shader.Get(), nullptr, 0);
//    dx.device_context->VSSetConstantBuffers(0, 1, object_constant_buffer.GetAddressOf());
//
//    dx.device_context->PSSetShader(pixel_shader->shader.Get(), nullptr, 0);
//    dx.device_context->PSSetConstantBuffers(0, 1, object_constant_buffer.GetAddressOf());
//
//    dx.device_context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());
//
//    // Draw the scene
//    for(const hstatic_mesh* sm : scene_acceleration->meshes)
//    {
//      const astatic_mesh* sma = sm->mesh_asset_ptr.get();
//      if(sma == nullptr) { continue; }
//      const fstatic_mesh_resource& smrs = sma->render_state;
//      const amaterial* ma = sm->material_asset_ptr.get();
//      if(ma == nullptr)
//      {
//        ma = default_material_asset.get();
//      }
//      const atexture* ta = ma->texture_asset_ptr.get();
//
//      // Update per-object constant buffer
//      {
//        XMMATRIX translation_matrix = XMMatrixTranslation(sm->origin.x, sm->origin.y, sm->origin.z);
//        XMMATRIX rotation_matrix = XMMatrixRotationX(XMConvertToRadians(sm->rotation.x))
//          * XMMatrixRotationY(XMConvertToRadians(sm->rotation.y))
//          * XMMatrixRotationZ(XMConvertToRadians(sm->rotation.z));
//        XMMATRIX scale_matrix = XMMatrixScaling(sm->scale.x, sm->scale.y, sm->scale.z);
//        XMMATRIX world_matrix = scale_matrix * rotation_matrix * translation_matrix;
//
//        const XMMATRIX inverse_transpose_model_world = XMMatrixTranspose(XMMatrixInverse(nullptr, world_matrix));
//        const XMMATRIX model_world_view_projection = XMMatrixMultiply(world_matrix, XMLoadFloat4x4(&scene->camera_config.view_projection));
//
//        fobject_data pod;
//        XMStoreFloat4x4(&pod.model_world, world_matrix);
//        XMStoreFloat4x4(&pod.inverse_transpose_model_world, inverse_transpose_model_world);
//        XMStoreFloat4x4(&pod.model_world_view_projection, model_world_view_projection);
//        if(const amaterial* material = sm->material_asset_ptr.get())
//        {
//          pod.material_id = scene_acceleration->material_map.at(material);
//        }
//        pod.is_selected = selected_object == sm ? 1 : 0;
//        pod.object_id = fmath::uint32_to_colorf(sm->get_hash());
//        dx.update_constant_buffer<fobject_data>(&pod, object_constant_buffer);
//      }
//
//      // Update texture
//      if(ma->properties.use_texture)
//      {
//        if(ta == nullptr)
//        {
//          ta = default_material_asset.get()->texture_asset_ptr.get();
//        }
//        dx.device_context->PSSetShaderResources(0, 1, ta->render_state.texture_srv.GetAddressOf());
//      }
//
//      // Update mesh
//      dx.device_context->IASetVertexBuffers(0, 1, smrs.vertex_buffer.GetAddressOf(), &smrs.stride, &smrs.offset);
//      dx.device_context->IASetIndexBuffer(smrs.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
//      static_assert(sizeof(fface_data_type) == sizeof(uint32_t));
//
//      // Draw
//      dx.device_context->DrawIndexed(smrs.num_faces, 0, 0);
//    }
  }

  void fgbuffer_pass::create_output_texture(bool cleanup)
  {
//    if(cleanup)
//    {
//      for(int i = 0; i < egbuffer_type::count; i++)
//      {
//        DX_RELEASE(output_rtv[i])
//        DX_RELEASE(output_srv[i])
//        DX_RELEASE(output_texture[i])
//      }
//      DX_RELEASE(output_dsv)
//      DX_RELEASE(output_depth)
//    }
//
//    fdx12& dx = fdx12::instance();
//    for(int i = 0; i < egbuffer_type::count; i++)
//    {
//      DXGI_FORMAT format = (i == egbuffer_type::material_id || i == egbuffer_type::is_selected ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32G32B32A32_FLOAT);
//      D3D11_BIND_FLAG bind_flag = static_cast<D3D11_BIND_FLAG>(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
//
//      dx.create_texture(output_width, output_height, format, bind_flag, D3D11_USAGE_DEFAULT, output_texture[i]);
//      dx.create_shader_resource_view(output_texture[i], format, D3D11_SRV_DIMENSION_TEXTURE2D, output_srv[i]);
//      dx.create_render_target_view(output_texture[i], format, D3D11_RTV_DIMENSION_TEXTURE2D, output_rtv[i]);
//    }
//
//    dx.create_texture(output_width, output_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, output_depth);
//    dx.create_depth_stencil_view(output_depth, output_width, output_height, output_dsv);
  }
}