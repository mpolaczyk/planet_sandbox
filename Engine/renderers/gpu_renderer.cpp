
#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "renderers/gpu_renderer.h"

#include "engine/log.h"

namespace engine
{
  OBJECT_DEFINE(gpu_renderer, async_renderer_base, GPU renderer)
  OBJECT_DEFINE_SPAWN(gpu_renderer)

  void gpu_renderer::job_init()
  {
    // FIX temporary hack here! It should be properly persistent as part fo the scene (not hittable)
    vertex_shader.set_name("gpu_renderer_vs");
    vertex_shader.get();
    pixel_shader.set_name("gpu_renderer_ps");
    pixel_shader.get();
    
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
  
  void gpu_renderer::job_update()
  {
    FLOAT bg_color[4] = { 0.1f, 0.5f, 0.7f, 1.0f };
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(job_state.image_width), static_cast<float>(job_state.image_height), 0.0f, 1.0f };
    
    dx11& dx = dx11::instance();
    dx.create_output_texture(job_state.image_width, job_state.image_height);
    dx.device_context->RSSetViewports(1, &viewport);
    dx.device_context->OMSetRenderTargets(1, &dx.output_rtv, NULL);
    dx.device_context->ClearRenderTargetView(dx.output_rtv, bg_color);
    dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx.device_context->IASetInputLayout(input_layout);
    dx.device_context->VSSetShader(vertex_shader.get()->shader, nullptr, 0);
    dx.device_context->PSSetShader(pixel_shader.get()->shader, nullptr, 0);
    dx.device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
    dx.device_context->Draw(num_verts, 0);
  }

  void gpu_renderer::job_cleanup()
  {
    
  }
}
