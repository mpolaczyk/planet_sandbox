#include "stdafx.h"

#include <d3d11_1.h>
#include <winerror.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

namespace dx11
{
  ID3D11Device1* g_device = nullptr;
  ID3D11DeviceContext1* g_device_context = nullptr;
  IDXGISwapChain1* g_swap_chain = nullptr;
  ID3D11RenderTargetView* g_frame_buffer_view = nullptr;

  void create_render_target()
  {
    ID3D11Texture2D* frame_buffer;
    HRESULT result = g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&frame_buffer));
    assert(SUCCEEDED(result));
    
    result = g_device->CreateRenderTargetView(frame_buffer, NULL, &g_frame_buffer_view);
    assert(SUCCEEDED(result));
    frame_buffer->Release();
  }

  bool create_device()
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
      LOG_CRITICAL("D3D11CreateDevice() failed.");
      const std::string error = win32_error::get_last_error_as_string();
      LOG_CRITICAL("{0}", error);
      return false;
    }

    result = base_device->QueryInterface(IID_PPV_ARGS(&g_device));
    if(FAILED(result))
    {
      LOG_CRITICAL("ID3D11Device1 query failed.");
      return false;
    }
    base_device->Release();

    result = base_device_context->QueryInterface(IID_PPV_ARGS(&g_device_context));
    if(FAILED(result))
    {
      LOG_CRITICAL("ID3D11DeviceContext1 query failed.");
      return false;
    }
    base_device_context->Release();
    return true;
  }

  bool create_debug_layer()
  {
#if BUILD_DEBUG
    ID3D11Debug *debug = nullptr;
    g_device->QueryInterface(IID_PPV_ARGS(&debug));
    if (debug)
    {
      ID3D11InfoQueue *info_queue = nullptr;
      if (SUCCEEDED(debug->QueryInterface(IID_PPV_ARGS(&info_queue))))
      {
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
        info_queue->Release();
      }
      else
      {
        LOG_CRITICAL("ID3D11InfoQueue query failed.");
        return false;
      }
      debug->Release();
    }
    else
    {
      LOG_CRITICAL("ID3D11Debug query failed.");
      return false;
    }
#endif
    return true;
  }

  bool create_swap_chain(HWND hwnd)
  {
    IDXGIFactory2* factory;
    {
      IDXGIDevice1* device;
      HRESULT result = g_device->QueryInterface(IID_PPV_ARGS(&device));
      assert(SUCCEEDED(result));

      IDXGIAdapter* adapter;
      result = device->GetAdapter(&adapter);
      assert(SUCCEEDED(result));  
      device->Release();

      DXGI_ADAPTER_DESC adapter_desc;
      adapter->GetDesc(&adapter_desc);

      OutputDebugStringA("Graphics Device: ");
      OutputDebugStringW(adapter_desc.Description);
      
      result = adapter->GetParent(IID_PPV_ARGS(&factory));
      assert(SUCCEEDED(result));
      adapter->Release();
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
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT result = factory->CreateSwapChainForHwnd(g_device, hwnd, &swap_chain_desc, 0, 0, &g_swap_chain);
    assert(SUCCEEDED(result));

    factory->Release();
    return true;
  }

  void cleanup_render_target()
  {
    if (g_frame_buffer_view) { g_frame_buffer_view->Release(); g_frame_buffer_view = NULL; }
  }

  void cleanup_device()
  {
    cleanup_render_target();
    if (g_swap_chain) { g_swap_chain->Release(); g_swap_chain = NULL; }
    if (g_device_context) { g_device_context->Release(); g_device_context = NULL; }
    if (g_device) { g_device->Release(); g_device = NULL; }
  }

  bool load_texture_from_buffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
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
    if (SUCCEEDED(g_device->CreateTexture2D(&desc, sub_resource, &texture)) && texture)
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
      ZeroMemory(&srv_desc, sizeof(srv_desc));
      srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srv_desc.Texture2D.MipLevels = desc.MipLevels;
      srv_desc.Texture2D.MostDetailedMip = 0;
      if (SUCCEEDED(g_device->CreateShaderResourceView(texture, &srv_desc, out_srv)))
      {
        *out_texture = texture;
        texture->Release();
        return true;
      }
    }
    return false;
  }

  bool update_texture_buffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture)
  {
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    ZeroMemory(&mapped_resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    int row_span = width * 4; // 4 bytes per px
    g_device_context->Map(in_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
    BYTE* mapped_data = reinterpret_cast<BYTE*>(mapped_resource.pData);
    for (int i = 0; i < height; ++i)
    {
      memcpy(mapped_data, buffer, row_span);
      mapped_data += mapped_resource.RowPitch;
      buffer += row_span;
    }
    g_device_context->Unmap(in_texture, 0);
    return true;
  }

  bool lod_texture_from_file(const char* filename, int& out_width, int& out_height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
  {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data != NULL)
    {
      out_width = image_width;
      out_height = image_height;
      bool answer = load_texture_from_buffer(image_data, image_width, image_height, out_srv, out_texture);
      stbi_image_free(image_data);
      return answer;
    }
    return true;
  }
}