
#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "renderers/gpu_renderer.h"

#include "engine/log.h"

namespace engine
{
  OBJECT_DEFINE(gpu_renderer, renderer_base, GPU renderer)
  OBJECT_DEFINE_SPAWN(gpu_renderer)

  void gpu_renderer::render_frame(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config)
  {
    if(output_width != in_renderer_config.resolution_vertical || output_height != in_renderer_config.resolution_horizontal)
    {
      DX_RELEASE(output_rtv)
      DX_RELEASE(output_srv)
      DX_RELEASE(output_texture)
    }
    output_width = in_renderer_config.resolution_horizontal;
    output_height = in_renderer_config.resolution_vertical;

    static bool init_done = false;
    if(!init_done) { init(); }
    
    update_frame();
  }
  
  void gpu_renderer::create_output_texture()
  {
    dx11& dx = dx11::instance();
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = output_width;
      desc.Height = output_height;
      desc.MipLevels = 1;
      desc.ArraySize = 1;
      desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      desc.SampleDesc.Count = 1;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      dx.device->CreateTexture2D(&desc, NULL, &output_texture);
    }
    {
      D3D11_RENDER_TARGET_VIEW_DESC desc = {};
      desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MipSlice = 0;
      HRESULT result = dx.device->CreateRenderTargetView(output_texture, &desc, &output_rtv);
      assert(SUCCEEDED(result));
    }
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
      desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MostDetailedMip = 0;
      desc.Texture2D.MipLevels = 1;
      HRESULT result = dx.device->CreateShaderResourceView(output_texture, &desc, &output_srv);
      assert(SUCCEEDED(result));
    }
  }
  
  void gpu_renderer::init()
  {
    // FIX temporary hack here! It should be properly persistent as part fo the scene (not hittable)
    vertex_shader.set_name("gpu_renderer_vs");
    vertex_shader.get();
    pixel_shader.set_name("gpu_renderer_ps");
    pixel_shader.get();
    
    create_output_texture();
    
    {
      D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
      {
        { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
      };
      const auto blob = vertex_shader.get()->shader_blob;
      HRESULT result = dx11::instance().device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc),
        blob->GetBufferPointer(), blob->GetBufferSize(), &input_layout);
      if (FAILED(result))
      {
        LOG_ERROR("Unable to create input layout.");
      }
    }  

    // FIX hardcoded mesh!
    {
      float vertex_data[] = { // x, y, r, g, b, a
        0.0f,  0.5f, 0.f, 1.f, 0.f, 1.f,
        0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f };
      stride = 6 * sizeof(float);
      num_verts = sizeof(vertex_data) / stride;
      offset = 0;

      D3D11_BUFFER_DESC vertex_buffer_desc = {};
      vertex_buffer_desc.ByteWidth = sizeof(vertex_data);
      vertex_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
      vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      D3D11_SUBRESOURCE_DATA vertex_subresource_data = { vertex_data };

      HRESULT result = dx11::instance().device->CreateBuffer(&vertex_buffer_desc, &vertex_subresource_data, &vertex_buffer);
      assert(SUCCEEDED(result));
    }
  }
  
  void gpu_renderer::update_frame()
  {
    FLOAT bg_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f };
    
    dx11& dx = dx11::instance();
    dx.device_context->RSSetViewports(1, &viewport);
    dx.device_context->OMSetRenderTargets(1, &output_rtv, NULL);
    dx.device_context->ClearRenderTargetView(output_rtv, bg_color);
    dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx.device_context->IASetInputLayout(input_layout);
    dx.device_context->VSSetShader(vertex_shader.get()->shader, nullptr, 0);
    dx.device_context->PSSetShader(pixel_shader.get()->shader, nullptr, 0);
    dx.device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
    dx.device_context->Draw(num_verts, 0);
  }

  void gpu_renderer::cleanup()
  {
    renderer_base::cleanup();
    DX_RELEASE(output_rtv)
    DX_RELEASE(input_layout)
    DX_RELEASE(vertex_buffer)
  }
}
