
#include <d3d11_1.h>
#include <winerror.h>
#include <cassert>

#include "renderer/dx11_lib.h"
#include "engine/log.h"
#include "engine/tools.h"

namespace engine
{
  void fdx11::create_device()
  {
    ComPtr<ID3D11Device> base_device;
    ComPtr<ID3D11DeviceContext> base_device_context;
    const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
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
    
    LOG_INFO("Graphics Device: {0}", ftools::to_utf8(adapter_desc.Description));

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

  void fdx11::create_render_target()
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
  
  bool fdx11::create_index_buffer(const std::vector<fface_data>& in_face_list, ComPtr<ID3D11Buffer>& out_index_buffer)
  {
    ComPtr<ID3D11Device1> device = fdx11::instance().device;
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = static_cast<int32_t>(in_face_list.size()) * sizeof(fface_data);
    desc.Usage     = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA data = { in_face_list.data() };
    return SUCCEEDED(device->CreateBuffer(&desc, &data, out_index_buffer.GetAddressOf()));
  }

  bool fdx11::create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, ComPtr<ID3D11Buffer>& out_vertex_buffer)
  {
    ComPtr<ID3D11Device1> device = fdx11::instance().device;
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = static_cast<int32_t>(in_vertex_list.size()) * sizeof(fvertex_data);
    desc.Usage     = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA data = { in_vertex_list.data() };
    return SUCCEEDED(device->CreateBuffer(&desc, &data, out_vertex_buffer.GetAddressOf()));
  }
  
  bool fdx11::create_texture_from_buffer(unsigned char* in_buffer, int width, int height, ComPtr<ID3D11ShaderResourceView>& out_srv, ComPtr<ID3D11Texture2D>& out_texture)
  {
    if (in_buffer == nullptr) return false;
    ComPtr<ID3D11Device1> device = fdx11::instance().device;
    
    // Texture
    ComPtr<ID3D11Texture2D> texture;
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = width;
      desc.Height = height;
      desc.MipLevels = 1;
      desc.ArraySize = 1;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.Usage = D3D11_USAGE_DYNAMIC;
      desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      D3D11_SUBRESOURCE_DATA sub_resource = {};
      sub_resource.pSysMem = in_buffer;
      sub_resource.SysMemPitch = desc.Width * 4;
      sub_resource.SysMemSlicePitch = 0;
      if (FAILED(device->CreateTexture2D(&desc, &sub_resource, texture.GetAddressOf())))
      {
        throw std::runtime_error("CreateTexture2D failed.");
      }
    }

    // SRV
    ComPtr<ID3D11ShaderResourceView> srv;
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MipLevels = 1;
      desc.Texture2D.MostDetailedMip = 0;
      if (FAILED(device->CreateShaderResourceView(texture.Get(), &desc, srv.GetAddressOf())))
      {
        throw std::runtime_error("CreateShaderResourceView failed.");
      }
    }

    out_srv = srv;
    out_texture = texture;
    
    return true;
  }

  bool fdx11::update_texture_from_buffer(unsigned char* in_buffer, int width, int height, ComPtr<ID3D11Texture2D>& out_texture)
  {
    if (in_buffer == nullptr) return false;
    ComPtr<ID3D11DeviceContext1> device_context = fdx11::instance().device_context;
    
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    ZeroMemory(&mapped_resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    int row_span = width * 4; // 4 bytes per px
    device_context->Map(out_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
    BYTE* mapped_data = reinterpret_cast<BYTE*>(mapped_resource.pData);
    for (int i = 0; i < height; ++i)
    {
      memcpy(mapped_data, in_buffer, row_span);
      mapped_data += mapped_resource.RowPitch;
      in_buffer += row_span;
    }
    device_context->Unmap(out_texture.Get(), 0);
    return true;
  }
}
