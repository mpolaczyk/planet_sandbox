#include "deferred_lighting_pass.h"

#include <vector>

#include "core/core.h"
#include "hittables/scene.h"
#include "math/vertex_data.h"
#include "renderer/aligned_structs.h"
#include "renderer/scene_acceleration.h"

namespace engine
{
  using namespace DirectX;

  namespace
  {
    ALIGNED_STRUCT_BEGIN(fframe_data)
    {
      DirectX::XMFLOAT4 camera_position; // 16
      XMFLOAT4 ambient_light; // 16
      XMFLOAT4X4 model_world_view_projection; // 64
      int32_t show_position_ws; // 4    // TODO pack bits
      int32_t show_normal_ws; // 4
      int32_t show_tex_color; // 4
      int32_t show_object_id; // 4
      flight_properties lights[MAX_LIGHTS]; // 80xN
      fmaterial_properties materials[MAX_MATERIALS]; // 80xN
    };

    ALIGNED_STRUCT_END(fframe_data)
  }

  void fdeferred_lighting_pass::init()
  {
//    fdx12& dx = fdx12::instance();
//    {
//      D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
//      {
//        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
//        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
//        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
//        {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
//        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
//      };
//      dx.create_input_layout(input_element_desc, ARRAYSIZE(input_element_desc), vertex_shader->blob, input_layout);
//    }
//    dx.create_sampler_state(sampler_state);
//    dx.create_constant_buffer(sizeof(fframe_data), frame_constant_buffer);
//    dx.create_rasterizer_state(rasterizer_state);
//    dx.create_depth_stencil_state(depth_stencil_state);
//
//    // Define quad
//    std::vector<fvertex_data> vertex_list;
//    vertex_list.reserve(4);
//    for(int i = 0; i < 4; i++)
//    {
//      vertex_list.push_back(fvertex_data());
//    }
//    vertex_list[0].position = {-1.0f, -1.0f, 0.0f};
//    vertex_list[1].position = {-1.0f, 1.0f, 0.0f};
//    vertex_list[2].position = {1.0f, 1.0f, 0.0f};
//    vertex_list[3].position = {1.0f, -1.0f, 0.0f};
//    vertex_list[0].normal = {0.0f, 0.0f, -1.0f};
//    vertex_list[1].normal = {0.0f, 0.0f, -1.0f};
//    vertex_list[2].normal = {0.0f, 0.0f, -1.0f};
//    vertex_list[3].normal = {0.0f, 0.0f, -1.0f};
//    vertex_list[0].tangent = {1.0f, 0.0f, 0.0f};
//    vertex_list[1].tangent = {1.0f, 0.0f, 0.0f};
//    vertex_list[2].tangent = {1.0f, 0.0f, 0.0f};
//    vertex_list[3].tangent = {1.0f, 0.0f, 0.0f};
//    vertex_list[0].uv = {0.0f, 1.0f};
//    vertex_list[1].uv = {0.0f, 0.0f};
//    vertex_list[2].uv = {1.0f, 0.0f};
//    vertex_list[3].uv = {1.0f, 1.0f};
//    fdx12::instance().create_vertex_buffer(vertex_list, quad_render_state.vertex_buffer);
//    quad_render_state.offset = 0;
//    quad_render_state.stride = sizeof(fvertex_data);
//
//    std::vector<fface_data> face_list;
//    face_list.reserve(2);
//    for(int i = 0; i < 2; i++)
//    {
//      face_list.push_back(fface_data());
//    }
//    face_list[0] = {0, 1, 2};
//    face_list[1] = {0, 2, 3};
//    fdx12::instance().create_index_buffer(face_list, quad_render_state.index_buffer);
//    quad_render_state.num_faces = static_cast<int32_t>(face_list.size()) * 3;
  }
  
  void fdeferred_lighting_pass::draw(const ComPtr<ID3D12GraphicsCommandList>& command_list)
  {
//    fdx12& dx = fdx12::instance();
//    
//    dx.device_context->ClearRenderTargetView(output_rtv.Get(), scene->clear_color);
//    dx.device_context->ClearDepthStencilView(output_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
//
//    const D3D11_VIEWPORT viewport = {0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f};
//    dx.device_context->RSSetViewports(1, &viewport);
//    dx.device_context->RSSetState(rasterizer_state.Get());
//    dx.device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);
//    dx.device_context->OMSetRenderTargets(1, output_rtv.GetAddressOf(), output_dsv.Get());
//    
//    dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    dx.device_context->IASetInputLayout(input_layout.Get());
//
//    dx.device_context->VSSetShader(vertex_shader->shader.Get(), nullptr, 0);
//    dx.device_context->VSSetConstantBuffers(0, 1, frame_constant_buffer.GetAddressOf());
//    
//    dx.device_context->PSSetShader(pixel_shader->shader.Get(), nullptr, 0);
//    dx.device_context->PSSetConstantBuffers(0, 1, frame_constant_buffer.GetAddressOf());
//    
//    const XMMATRIX model_world_view_projection = XMMatrixIdentity();
//    
//    // Update per-frame constant buffer
//    {
//      fframe_data pfd;
//      scene_acceleration->get_lights_array(pfd.lights);
//      scene_acceleration->get_materials_array(pfd.materials);
//      XMStoreFloat4x4(&pfd.model_world_view_projection, model_world_view_projection); // TODO REMOVE!
//      pfd.camera_position = XMFLOAT4(scene->camera_config.location.e);
//      pfd.ambient_light = scene->ambient_light_color;
//      pfd.show_normal_ws = show_normal_ws;
//      pfd.show_position_ws = show_position_ws;
//      pfd.show_tex_color = show_tex_color;
//      pfd.show_object_id = show_object_id;
//      dx.update_constant_buffer<fframe_data>(&pfd, frame_constant_buffer);
//    }
//
//    for(int i = 0; i < egbuffer_type::count; i++)
//    {
//      dx.device_context->PSSetShaderResources(i, 1, gbuffer_srvs[i].GetAddressOf());
//    }
//
//    dx.device_context->IASetVertexBuffers(0, 1, quad_render_state.vertex_buffer.GetAddressOf(), &quad_render_state.stride, &quad_render_state.offset);
//    dx.device_context->IASetIndexBuffer(quad_render_state.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//    dx.device_context->DrawIndexed(quad_render_state.num_faces, 0, 0);
//
//    // Do this to prevent:
//    // WARNING: ID3D11DeviceContext::OMSetRenderTargets: Resource being set to OM RenderTarget slot 0 is still bound on input! [ STATE_SETTING WARNING #9: DEVICE_OMSETRENDERTARGETS_HAZARD]
//    // In the second frame
//    for(int i = 0; i < egbuffer_type::count; i++)
//    {
//      ID3D11ShaderResourceView* null_srv = nullptr;
//      dx.device_context->PSSetShaderResources(i, 1, &null_srv);
//    }
  }
  
  void fdeferred_lighting_pass::create_output_texture(bool cleanup)
  {
    //    if(cleanup)
    //    {
    //      DX_RELEASE(output_rtv)
    //      DX_RELEASE(output_srv)
    //      DX_RELEASE(output_dsv)
    //      DX_RELEASE(output_texture)
    //      DX_RELEASE(output_depth)
    //    }
    //
    //    fdx12& dx = fdx12::instance();
    //    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //    D3D11_BIND_FLAG bind_flag = static_cast<D3D11_BIND_FLAG>(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    //    dx.create_texture(output_width, output_height, format, bind_flag, D3D11_USAGE_DEFAULT, output_texture);
    //    dx.create_shader_resource_view(output_texture, format, D3D11_SRV_DIMENSION_TEXTURE2D, output_srv);
    //    dx.create_render_target_view(output_texture, format, D3D11_RTV_DIMENSION_TEXTURE2D, output_rtv);
    //
    //    dx.create_texture(output_width, output_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, output_depth);
    //    dx.create_depth_stencil_view(output_depth, output_width, output_height, output_dsv);
  }
}