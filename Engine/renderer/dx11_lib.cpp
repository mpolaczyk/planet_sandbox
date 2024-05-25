#include <d3d11_1.h>
#include <winerror.h>
#include <cassert>

#include "renderer/dx11_lib.h"
#include "engine/log.h"
#include "engine/string_tools.h"

namespace engine
{
  void fdx11::create_device()
  {
    ComPtr<ID3D11Device> base_device;
    ComPtr<ID3D11DeviceContext> base_device_context;
    const D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
    UINT flags = 0;
#if BUILD_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    if(FAILED(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
      0, flags,
      feature_levels, 2,
      D3D11_SDK_VERSION, base_device.GetAddressOf(),
      0, base_device_context.GetAddressOf())))
    {
      throw std::runtime_error("D3D11CreateDevice failed.");
    }

    if(FAILED(base_device.As<ID3D11Device1>(&device)))
    {
      throw std::runtime_error("ID3D11Device1 query failed.");
    }

    if(FAILED(base_device_context.As<ID3D11DeviceContext1>(&device_context)))
    {
      throw std::runtime_error("ID3D11DeviceContext1 query failed.");
    }
  }

  void fdx11::create_debug_layer() const
  {
#if BUILD_DEBUG
    ComPtr<ID3D11Debug> debug;
    if(FAILED(device.As<ID3D11Debug>(&debug)))
    {
      throw std::runtime_error("ID3D11Debug query failed.");
    }
    
    ComPtr<ID3D11InfoQueue> info_queue;
    if (FAILED(debug.As<ID3D11InfoQueue>(&info_queue)))
    {
      throw std::runtime_error("ID3D11InfoQueue query failed.");
    }
    
    info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
    info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
    info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
#endif
  }

  void fdx11::create_swap_chain(HWND hwnd)
  {
    ComPtr<IDXGIDevice1> dxgi_device;
    if(FAILED(device.As<IDXGIDevice1>(&dxgi_device)))
    {
      throw std::runtime_error("IDXGIDevice query failed.");
    }

    ComPtr<IDXGIAdapter> adapter;
    if(FAILED(dxgi_device->GetAdapter(&adapter)))
    {
      throw std::runtime_error("GetAdapter failed.");
    }

    DXGI_ADAPTER_DESC adapter_desc;
    adapter->GetDesc(&adapter_desc);

    LOG_INFO("Graphics Device: {0}", fstring_tools::to_utf8(adapter_desc.Description));

    ComPtr<IDXGIFactory2> factory;
    if(FAILED(adapter->GetParent(IID_PPV_ARGS(factory.GetAddressOf()))))
    {
      throw std::runtime_error("GetParent failed.");
    }

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Width = 0;
    swap_chain_desc.Height = 0;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    if(FAILED(factory->CreateSwapChainForHwnd(device.Get(), hwnd, &swap_chain_desc, 0, 0, swap_chain.GetAddressOf())))
    {
      throw std::runtime_error("CreateSwapChainForHwnd failed.");
    }
  }

  void fdx11::create_render_target()  // TODO overlaps with create_render_target_view(), clean it up!
  {
    ComPtr<ID3D11Texture2D> frame_buffer;
    if(FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&frame_buffer))))
    {
      throw std::runtime_error("GetBuffer frame buffer failed.");
    }

    if(FAILED(device->CreateRenderTargetView(frame_buffer.Get(), nullptr, rtv.GetAddressOf())))
    {
      throw std::runtime_error("CreateRenderTargetView failed.");
    }
  }

  void fdx11::create_input_layout(const D3D11_INPUT_ELEMENT_DESC* input_element_desc, uint32_t input_element_desc_size, const ComPtr<ID3D10Blob>& vertex_shader_blob, ComPtr<ID3D11InputLayout>& input_layout) const
  {
    if(FAILED(device->CreateInputLayout(input_element_desc, input_element_desc_size, vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), input_layout.GetAddressOf())))
    {
      throw std::runtime_error("CreateInputLayout failed.");
    }
  }

  void fdx11::create_sampler_state(ComPtr<ID3D11SamplerState>& out_sampler_state) const
  {
    D3D11_SAMPLER_DESC desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    desc.BorderColor[0] = 1.0f;
    desc.BorderColor[1] = 1.0f;
    desc.BorderColor[2] = 1.0f;
    desc.BorderColor[3] = 1.0f;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    if(FAILED(device->CreateSamplerState(&desc, &out_sampler_state)))
    {
      throw std::runtime_error("CreateSamplerState failed.");
    }
  }

  void fdx11::create_constant_buffer(uint32_t size, ComPtr<ID3D11Buffer>& out_constant_buffer) const
  {
    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.ByteWidth = size;
    if(FAILED(device->CreateBuffer(&desc, nullptr, &out_constant_buffer)))
    {
      throw std::runtime_error("CreateBuffer failed.");
    }
  }

  void fdx11::create_rasterizer_state(ComPtr<ID3D11RasterizerState>& out_rasterizer_state) const
  {
    D3D11_RASTERIZER_DESC desc = {};
    desc.AntialiasedLineEnable = FALSE;
    desc.CullMode = D3D11_CULL_BACK;
    desc.DepthBias = 0;
    desc.DepthBiasClamp = 0.0f;
    desc.DepthClipEnable = TRUE;
    desc.FillMode = D3D11_FILL_SOLID;
    desc.FrontCounterClockwise = FALSE;
    desc.MultisampleEnable = FALSE;
    desc.ScissorEnable = FALSE;
    desc.SlopeScaledDepthBias = 0.0f;
    if(FAILED(device->CreateRasterizerState(&desc, &out_rasterizer_state)))
    {
      throw std::runtime_error("CreateRasterizerState failed.");
    }
  }

  void fdx11::create_depth_stencil_state(ComPtr<ID3D11DepthStencilState>& out_depth_stencil_state) const
  {
    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = TRUE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS;
    desc.StencilEnable = FALSE;
    if(FAILED(device->CreateDepthStencilState(&desc, &out_depth_stencil_state)))
    {
      throw std::runtime_error("CreateDepthStencilState failed.");
    }
  }

  void fdx11::create_vertex_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11VertexShader>& out_vertex_shader) const
  {
    if(FAILED(device->CreateVertexShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, out_vertex_shader.GetAddressOf())))
    {
      throw std::runtime_error("CreateVertexShader failed");
    }
  }

  void fdx11::create_pixel_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11PixelShader>& out_pixel_shader) const
  {
    if(FAILED(device->CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, out_pixel_shader.GetAddressOf())))
    {
      throw std::runtime_error("CreatePixelShader failed");
    }
  }

  void fdx11::create_index_buffer(const std::vector<fface_data>& in_face_list, ComPtr<ID3D11Buffer>& out_index_buffer) const
  {
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = static_cast<int32_t>(in_face_list.size()) * sizeof(fface_data);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA data = {in_face_list.data()};
    if(FAILED(device->CreateBuffer(&desc, &data, out_index_buffer.GetAddressOf())))
    {
      throw std::runtime_error("CreateBuffer index failed.");
    }
  }

  void fdx11::create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, ComPtr<ID3D11Buffer>& out_vertex_buffer) const
  {
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = static_cast<int32_t>(in_vertex_list.size()) * sizeof(fvertex_data);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    const D3D11_SUBRESOURCE_DATA data = {in_vertex_list.data()};
    if(FAILED(device->CreateBuffer(&desc, &data, out_vertex_buffer.GetAddressOf())))
    {
      throw std::runtime_error("CreateBuffer vertex failed.");
    }
  }

  void fdx11::create_render_target_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_RTV_DIMENSION view_dimmension, ComPtr<ID3D11RenderTargetView>& out_render_target_view) const
  {
    D3D11_RENDER_TARGET_VIEW_DESC desc = {};
    desc.Format = format;
    desc.ViewDimension = view_dimmension;
    desc.Texture2D.MipSlice = 0;
    if(FAILED(device->CreateRenderTargetView(in_texture.Get(), &desc, out_render_target_view.GetAddressOf())))
    {
      throw std::runtime_error("CreateRenderTargetView failed");
    }
  }

  void fdx11::create_depth_stencil_view(const ComPtr<ID3D11Texture2D>& in_texture, uint32_t width, uint32_t height, ComPtr<ID3D11DepthStencilView>& out_depth_stencil_view) const
  {
    if(FAILED(device->CreateDepthStencilView(in_texture.Get(), nullptr, out_depth_stencil_view.GetAddressOf()))) // TODO Missing descriptor?
    {
      throw std::runtime_error("CreateDepthStencilView output depth texture failed");
    }
  }

  void fdx11::create_texture(uint32_t width, uint32_t height, DXGI_FORMAT format, D3D11_BIND_FLAG bind_flags, D3D11_USAGE usage, ComPtr<ID3D11Texture2D>& out_texture, uint32_t bytes_per_row, const void* in_bytes) const
  {
    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.Format = format;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = usage;
    texture_desc.BindFlags = bind_flags;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA texture_subresource_data = {};
    texture_subresource_data.SysMemPitch = bytes_per_row;
    texture_subresource_data.pSysMem = in_bytes;
    
    if(FAILED(device->CreateTexture2D(&texture_desc, in_bytes != nullptr ? &texture_subresource_data : nullptr, out_texture.GetAddressOf())))
    {
      throw std::runtime_error("CreateTexture2D failed.");
    }
  }
  
  void fdx11::create_shader_resource_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_SRV_DIMENSION view_dimmension, ComPtr<ID3D11ShaderResourceView>& out_shader_resource_view) const
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
    shader_resource_view_desc.Format = format;
    shader_resource_view_desc.ViewDimension = view_dimmension;
    shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
    shader_resource_view_desc.Texture2D.MipLevels = 1;
    if(FAILED(device->CreateShaderResourceView(in_texture.Get(), &shader_resource_view_desc, out_shader_resource_view.GetAddressOf())))
    {
      throw std::runtime_error("CreateShaderResourceView failed.");
    }
  }

  void fdx11::cleanup_render_target()
  {
    DX_RELEASE(rtv)
  }

  void fdx11::cleanup_device()
  {
    cleanup_render_target();
    DX_RELEASE(swap_chain)
    DX_RELEASE(device_context)
    DX_RELEASE(device)
  }
}
