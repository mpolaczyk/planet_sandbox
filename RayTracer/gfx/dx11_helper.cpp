#include "stdafx.h"

#include <d3d11_1.h>
#include <winerror.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

namespace dx11
{
  ID3D11Device1* g_pd3dDevice = nullptr;
  ID3D11DeviceContext1* g_pd3dDeviceContext = nullptr;
  IDXGISwapChain1* g_pSwapChain = nullptr;
  ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

  void CreateRenderTarget()
  {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
  }

  bool CreateDeviceD3D()
  {
    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;
    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    UINT flags = 0;
#if BUILD_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 
                                          0, flags, 
                                          featureLevels, 2, 
                                          D3D11_SDK_VERSION, &baseDevice, 
                                          0, &baseDeviceContext);
    if (FAILED(hResult))
    {
      LOG_CRITICAL("D3D11CreateDevice() failed.");
      const std::string error = win32_error::get_last_error_as_string();
      LOG_CRITICAL("{0}", error.c_str());
      return false;
    }

    hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&g_pd3dDevice);
    if(FAILED(hResult))
    {
      LOG_CRITICAL("ID3D11Device1 query failed.");
      return false;
    }
    baseDevice->Release();

    hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&g_pd3dDeviceContext);
    if(FAILED(hResult))
    {
      LOG_CRITICAL("ID3D11DeviceContext1 query failed.");
      return false;
    }
    baseDeviceContext->Release();
    return true;
  }

  bool CreateDebugLayer()
  {
#if BUILD_DEBUG
    ID3D11Debug *d3dDebug = nullptr;
    g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
    if (d3dDebug)
    {
      ID3D11InfoQueue *d3dInfoQueue = nullptr;
      if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
      {
        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
        d3dInfoQueue->Release();
      }
      else
      {
        LOG_CRITICAL("ID3D11InfoQueue query failed.");
        return false;
      }
      d3dDebug->Release();
    }
    else
    {
      LOG_CRITICAL("ID3D11Debug query failed.");
      return false;
    }
#endif
    return true;
  }

  bool CreateSwapChain(HWND hwnd)
  {
    IDXGIFactory2* dxgiFactory;
    {
      IDXGIDevice1* dxgiDevice;
      HRESULT hResult = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
      assert(SUCCEEDED(hResult));

      IDXGIAdapter* dxgiAdapter;
      hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
      assert(SUCCEEDED(hResult));
      dxgiDevice->Release();

      DXGI_ADAPTER_DESC adapterDesc;
      dxgiAdapter->GetDesc(&adapterDesc);

      OutputDebugStringA("Graphics Device: ");
      OutputDebugStringW(adapterDesc.Description);
      
      hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
      assert(SUCCEEDED(hResult));
      dxgiAdapter->Release();
    }

    DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
    d3d11SwapChainDesc.Width = 0;
    d3d11SwapChainDesc.Height = 0;
    d3d11SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3d11SwapChainDesc.SampleDesc.Count = 1;
    d3d11SwapChainDesc.SampleDesc.Quality = 0;
    d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    d3d11SwapChainDesc.BufferCount = 2;
    d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    d3d11SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(g_pd3dDevice, hwnd, &d3d11SwapChainDesc, 0, 0, &g_pSwapChain);
    assert(SUCCEEDED(hResult));

    dxgiFactory->Release();
    return true;
  }

  void CleanupRenderTarget()
  {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
  }

  void CleanupDeviceD3D()
  {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
  }

  bool LoadTextureFromBuffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
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

    D3D11_SUBRESOURCE_DATA* subResource = new D3D11_SUBRESOURCE_DATA();
    subResource->pSysMem = buffer;
    subResource->SysMemPitch = desc.Width * 4;
    subResource->SysMemSlicePitch = 0;

    ID3D11Texture2D* pTexture = nullptr;
    if (SUCCEEDED(g_pd3dDevice->CreateTexture2D(&desc, subResource, &pTexture)) && pTexture)
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
      ZeroMemory(&srvDesc, sizeof(srvDesc));
      srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = desc.MipLevels;
      srvDesc.Texture2D.MostDetailedMip = 0;
      if (SUCCEEDED(g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv)))
      {
        *out_texture = pTexture;
        pTexture->Release();
        return true;
      }
    }
    return false;
  }

  bool UpdateTextureBuffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture)
  {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    int rowspan = width * 4; // 4 bytes per px
    g_pd3dDeviceContext->Map(in_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
    for (int i = 0; i < height; ++i)
    {
      memcpy(mappedData, buffer, rowspan);
      mappedData += mappedResource.RowPitch;
      buffer += rowspan;
    }
    g_pd3dDeviceContext->Unmap(in_texture, 0);
    return true;
  }

  bool LoadTextureFromFile(const char* filename, int& out_width, int& out_height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
  {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data != NULL)
    {
      out_width = image_width;
      out_height = image_height;
      bool answer = LoadTextureFromBuffer(image_data, image_width, image_height, out_srv, out_texture);
      stbi_image_free(image_data);
      return answer;
    }
    return true;
  }
}