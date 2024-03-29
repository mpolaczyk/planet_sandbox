
#include <d3d11_1.h>
#include <winerror.h>
#include <cassert>

#include "renderer/dx11_lib.h"
#include "engine/log.h"

namespace engine
{
  bool dx11::create_device()
  {
    ID3D11Device* base_device;
    ID3D11DeviceContext* base_device_context;
    const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    UINT flags = 0;
#if BUILD_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 
                                          0, flags, 
                                          feature_levels, 2, 
                                          D3D11_SDK_VERSION, &base_device, 
                                          0, &base_device_context);
    if (FAILED(result))
    {
      LOG_CRITICAL("D3D11CreateDevice failed.");
      //const std::string error = win32_error::get_last_error_as_string(); FIX
      //LOG_CRITICAL("{0}", error);
      return false;
    }

    result = base_device->QueryInterface(IID_PPV_ARGS(&device));
    if(FAILED(result))
    {
      LOG_CRITICAL("ID3D11Device1 query failed.");
      return false;
    }
    DX_RELEASE(base_device)

    result = base_device_context->QueryInterface(IID_PPV_ARGS(&device_context));
    if(FAILED(result))
    {
      LOG_CRITICAL("ID3D11DeviceContext1 query failed.");
      return false;
    }
    DX_RELEASE(base_device_context)
    return true;
  }

  bool dx11::create_debug_layer()
  {
#if BUILD_DEBUG
    ID3D11Debug *debug = nullptr;
    device->QueryInterface(IID_PPV_ARGS(&debug));
    if (debug)
    {
      ID3D11InfoQueue *info_queue = nullptr;
      if (SUCCEEDED(debug->QueryInterface(IID_PPV_ARGS(&info_queue))))
      {
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
        DX_RELEASE(info_queue)
      }
      else
      {
        LOG_CRITICAL("ID3D11InfoQueue query failed.");
        return false;
      }
      DX_RELEASE(debug)
    }
    else
    {
      LOG_CRITICAL("ID3D11Debug query failed.");
      return false;
    }
#endif
    return true;
  }

  bool dx11::create_swap_chain(HWND hwnd)
  {
    IDXGIFactory2* factory;
    {
      IDXGIDevice1* dxgi_device;
      HRESULT result = device->QueryInterface(IID_PPV_ARGS(&dxgi_device));
      assert(SUCCEEDED(result));

      IDXGIAdapter* adapter;
      result = dxgi_device->GetAdapter(&adapter);
      assert(SUCCEEDED(result));  
      DX_RELEASE(dxgi_device)

      DXGI_ADAPTER_DESC adapter_desc;
      adapter->GetDesc(&adapter_desc);

      OutputDebugStringA("Graphics Device: ");
      OutputDebugStringW(adapter_desc.Description);
      
      result = adapter->GetParent(IID_PPV_ARGS(&factory));
      assert(SUCCEEDED(result));
      DX_RELEASE(adapter)
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

    HRESULT result = factory->CreateSwapChainForHwnd(device, hwnd, &swap_chain_desc, 0, 0, &swap_chain);
    assert(SUCCEEDED(result));

    DX_RELEASE(factory)
    return true;
  }

  void dx11::create_render_target()
  {
    ID3D11Texture2D* frame_buffer;
    HRESULT result = swap_chain->GetBuffer(0, IID_PPV_ARGS(&frame_buffer));
    assert(SUCCEEDED(result));
    
    result = device->CreateRenderTargetView(frame_buffer, NULL, &rtv);
    assert(SUCCEEDED(result));
    DX_RELEASE(frame_buffer)
  }

  void dx11::cleanup_render_target()
  {
    DX_RELEASE(rtv)
  }

  void dx11::cleanup_device()
  {
    cleanup_render_target();
    DX_RELEASE(swap_chain)
    DX_RELEASE(device_context)
    DX_RELEASE(device)
  }

  bool dx11::load_texture_from_buffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
  {
    if (buffer == nullptr) return false;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA* sub_resource = new D3D11_SUBRESOURCE_DATA();
    sub_resource->pSysMem = buffer;
    sub_resource->SysMemPitch = desc.Width * 4;
    sub_resource->SysMemSlicePitch = 0;

    ID3D11Texture2D* texture = nullptr;
    if (SUCCEEDED(device->CreateTexture2D(&desc, sub_resource, &texture)) && texture)
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
      ZeroMemory(&srv_desc, sizeof(srv_desc));
      srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srv_desc.Texture2D.MipLevels = desc.MipLevels;
      srv_desc.Texture2D.MostDetailedMip = 0;
      if (SUCCEEDED(device->CreateShaderResourceView(texture, &srv_desc, out_srv)))
      {
        *out_texture = texture;
        return true;
      }
    }
    return false;
  }

  bool dx11::update_texture_buffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture)
  {
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    ZeroMemory(&mapped_resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    int row_span = width * 4; // 4 bytes per px
    device_context->Map(in_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
    BYTE* mapped_data = reinterpret_cast<BYTE*>(mapped_resource.pData);
    for (int i = 0; i < height; ++i)
    {
      memcpy(mapped_data, buffer, row_span);
      mapped_data += mapped_resource.RowPitch;
      buffer += row_span;
    }
    device_context->Unmap(in_texture, 0);
    return true;
  }
}