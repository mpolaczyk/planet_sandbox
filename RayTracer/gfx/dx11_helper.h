#pragma once

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace dx11
{
  extern ID3D11Device1* g_pd3dDevice;
  extern ID3D11DeviceContext1* g_pd3dDeviceContext;
  extern IDXGISwapChain1* g_pSwapChain;
  extern ID3D11RenderTargetView* g_mainRenderTargetView;

  bool CreateDeviceD3D();
  bool CreateDebugLayer();
  bool CreateSwapChain(HWND hwnd);
  void CreateRenderTarget();
  
  void CleanupDeviceD3D();
  void CleanupRenderTarget();
  
  bool UpdateTextureBuffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture);
  bool LoadTextureFromBuffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
  bool LoadTextureFromFile(const char* filename, int& out_width, int& out_height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
}