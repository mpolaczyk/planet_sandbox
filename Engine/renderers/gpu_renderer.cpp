
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
    if (in_renderer_config.resolution_vertical == 0 || in_renderer_config.resolution_horizontal == 0) return;

    camera = in_camera_config;
    
    const bool recreate_output_buffers = output_width != in_renderer_config.resolution_vertical || output_height != in_renderer_config.resolution_horizontal;
    output_width = in_renderer_config.resolution_horizontal;
    output_height = in_renderer_config.resolution_vertical;
    if(recreate_output_buffers)
    {
      create_output_texture(true);
    }
    
    if(!init_done) { init(); init_done = true; }
    
    update_frame();
  }
  
  void gpu_renderer::create_output_texture(bool cleanup)
  {
    if(cleanup)
    {
      DX_RELEASE(output_rtv)
      DX_RELEASE(output_srv)
      DX_RELEASE(output_texture)
    }
    dx11& dx = dx11::instance();
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = output_width;
      desc.Height = output_height;
      desc.MipLevels = 1;
      desc.ArraySize = 1;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      HRESULT result = dx.device->CreateTexture2D(&desc, NULL, &output_texture);
      assert(SUCCEEDED(result));
      
      desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
      desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
      result = dx.device->CreateTexture2D(&desc, NULL, &output_depth_texture);
      assert(SUCCEEDED(result));
    }
    {
      D3D11_RENDER_TARGET_VIEW_DESC desc = {};
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MipSlice = 0;
      HRESULT result = dx.device->CreateRenderTargetView(output_texture, &desc, &output_rtv);
      assert(SUCCEEDED(result));

      result = dx.device->CreateDepthStencilView(output_depth_texture, nullptr, &output_dsv);
      assert(SUCCEEDED(result));
    }
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MostDetailedMip = 0;
      desc.Texture2D.MipLevels = 1;
      HRESULT result = dx.device->CreateShaderResourceView(output_texture, &desc, &output_srv);
      assert(SUCCEEDED(result));
    }
  }
  
  void gpu_renderer::init()
  {
    // FIX temporary hack here! It should be properly persistent as part for the scene (not hittable)
    vertex_shader_asset.set_name("gpu_renderer_vs");
    vertex_shader_asset.get();
    pixel_shader_asset.set_name("gpu_renderer_ps");
    pixel_shader_asset.get();
    texture_asset.set_name("box");
    texture_asset.get();
    mesh_asset.set_name("cube24");
    mesh_asset.get();
    
    create_output_texture();

    auto device = dx11::instance().device;

    // Measuring time
    {
      timestamp_start = 0;
      perf_counter_frequency = 0;
      {
        LARGE_INTEGER perf_count;
        QueryPerformanceCounter(&perf_count);
        timestamp_start = perf_count.QuadPart;
        LARGE_INTEGER perf_freq;
        QueryPerformanceFrequency(&perf_freq);
        perf_counter_frequency = perf_freq.QuadPart;
      }
      current_time = 0.0;
    }
    
    {
      // Input layout
      D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
      {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }  // FIX add normal
      };
      const auto blob = vertex_shader_asset.get()->shader_blob;
      HRESULT result = device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), blob->GetBufferPointer(), blob->GetBufferSize(), &input_layout);
      assert(SUCCEEDED(result));
    }  

    // Static mesh asset
    {
      const static_mesh_asset* asset = mesh_asset.get();
      stride = sizeof(vertex_data);
      offset = 0;
      num_indices = asset->index_buffer.size();
      {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = asset->vertex_buffer.size() * sizeof(vertex_data);
        desc.Usage     = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA data = { asset->vertex_buffer.data() };

        HRESULT result = device->CreateBuffer(&desc, &data, &vertex_buffer);
        assert(SUCCEEDED(result));
      }
      {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = asset->index_buffer.size() * sizeof(uint16_t);
        desc.Usage     = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA data = { asset->index_buffer.data() };

        HRESULT result = device->CreateBuffer(&desc, &data, &index_buffer);
        assert(SUCCEEDED(result));
      }
    }
    
    // Create Sampler State
    {
      D3D11_SAMPLER_DESC desc = {};
      desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
      desc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
      desc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
      desc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
      desc.BorderColor[0] = 1.0f;
      desc.BorderColor[1] = 1.0f;
      desc.BorderColor[2] = 1.0f;
      desc.BorderColor[3] = 1.0f;
      desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    
      HRESULT result = device->CreateSamplerState(&desc, &sampler_state);
      assert(SUCCEEDED(result));
    }

    // Load texture asset
    {
      const int texture_bytes_per_row = 4 * texture_asset.get()->width;
      const uint8_t* texture_bytes = texture_asset.get()->data_ldr;
      assert(texture_bytes);
    
      D3D11_SUBRESOURCE_DATA texture_subresource_data = {};
      texture_subresource_data.SysMemPitch = texture_bytes_per_row;
      texture_subresource_data.pSysMem = texture_bytes;
    
      D3D11_TEXTURE2D_DESC texture_desc = {};
      texture_desc.Width              = texture_asset.get()->width;
      texture_desc.Height             = texture_asset.get()->height;
      texture_desc.MipLevels          = 1;
      texture_desc.ArraySize          = 1;
      texture_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
      texture_desc.SampleDesc.Count   = 1;
      texture_desc.Usage              = D3D11_USAGE_IMMUTABLE;
      texture_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    
      ID3D11Texture2D* texture;
      HRESULT result = device->CreateTexture2D(&texture_desc, &texture_subresource_data, &texture);
      assert(SUCCEEDED(result));
    
      result = device->CreateShaderResourceView(texture, nullptr, &texture_srv);
      assert(SUCCEEDED(result));
    }

    // Constant buffer
    {
      D3D11_BUFFER_DESC desc = {};
      // ByteWidth must be a multiple of 16, per the docs
      desc.ByteWidth      = sizeof(constants) + 0xf & 0xfffffff0;
      desc.Usage          = D3D11_USAGE_DYNAMIC;
      desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      HRESULT result = device->CreateBuffer(&desc, nullptr, &constant_buffer);
      assert(SUCCEEDED(result));
    }

    // Rasterizer state
    {
      D3D11_RASTERIZER_DESC desc = {};
      desc.FillMode = D3D11_FILL_SOLID;
      desc.CullMode = D3D11_CULL_BACK;
      desc.FrontCounterClockwise = TRUE;
      HRESULT result = device->CreateRasterizerState(&desc, &rasterizer_state);
      assert(SUCCEEDED(result));
    }

    // Depth stencil state
    {
      D3D11_DEPTH_STENCIL_DESC desc = {};
      desc.DepthEnable    = TRUE;
      desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
      desc.DepthFunc      = D3D11_COMPARISON_LESS;
      HRESULT result = device->CreateDepthStencilState(&desc, &depth_stencil_state);
      assert(SUCCEEDED(result));
    }
    
    float aspect_ratio = static_cast<float>(output_width) / static_cast<float>(output_height);

    constexpr float _rad = XMConvertToRadians(84);
    perspective_mat = XMMatrixPerspectiveFovRH(_rad, aspect_ratio, 0.1, 1000.f);
    camera_pos = {0.f, 0.f, -2.f};
    camera_rot = {0.f, 0.f, 0.f };
  }
  
  void gpu_renderer::update_frame()
  {
    float delta_time = 0.0f;
    {
      const double previous_time = current_time;
      LARGE_INTEGER perf_count;
      QueryPerformanceCounter(&perf_count);
      const LONGLONG timestamp_now = perf_count.QuadPart;
      current_time = static_cast<double>(timestamp_now - timestamp_start) / static_cast<double>(perf_counter_frequency);
      delta_time = static_cast<float>(current_time - previous_time);
    }
    
    FLOAT bg_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f }; // FIX use color from the library
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f };

    dx11& dx = dx11::instance();

    // Calculate view matrix from camera data
    // Negate the rotation to achieve inverse
    const XMMATRIX view_mat = XMMatrixMultiply(XMMatrixTranslationFromVector(camera_pos), XMMatrixRotationRollPitchYawFromVector(XMVectorScale(camera_rot, -1.f)));
    const float angle = static_cast<float>(math::pi * current_time);
    const XMVECTOR model_rot = { -0.2f * angle, 0.1f * angle, 0.f };
    const XMMATRIX model_mat = XMMatrixRotationRollPitchYawFromVector(model_rot);
    const XMMATRIX model_view_proj = XMMatrixMultiply(XMMatrixMultiply(model_mat, view_mat), perspective_mat);
    
    // Update constant buffer
    {
      D3D11_MAPPED_SUBRESOURCE mapped_subresource;
      dx.device_context->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
      constants* c = static_cast<constants*>(mapped_subresource.pData);
      c->model_view_proj = XMMatrixTranspose(model_view_proj);
      dx.device_context->Unmap(constant_buffer, 0);
    }
    
    dx.device_context->ClearRenderTargetView(output_rtv, bg_color);
    dx.device_context->ClearDepthStencilView(output_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

    dx.device_context->RSSetViewports(1, &viewport);
    dx.device_context->RSSetState(rasterizer_state);
    dx.device_context->OMSetDepthStencilState(depth_stencil_state, 0);
    dx.device_context->OMSetRenderTargets(1, &output_rtv, output_dsv);
    
    dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx.device_context->IASetInputLayout(input_layout);
    dx.device_context->VSSetShader(vertex_shader_asset.get()->shader, nullptr, 0);
    dx.device_context->PSSetShader(pixel_shader_asset.get()->shader, nullptr, 0);
    dx.device_context->PSSetShaderResources(0, 1, &texture_srv);
    dx.device_context->PSSetSamplers(0, 1, &sampler_state);
    dx.device_context->VSSetConstantBuffers(0, 1, &constant_buffer);
    dx.device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
    dx.device_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);
    
    dx.device_context->DrawIndexed(num_indices, 0, 0);
  }

  void gpu_renderer::cleanup()
  {
    renderer_base::cleanup();
    DX_RELEASE(output_rtv)
    DX_RELEASE(input_layout)
    DX_RELEASE(vertex_buffer)
  }
}
