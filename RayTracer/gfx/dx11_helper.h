#pragma once

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace dx11
{
  extern ID3D11Device1* g_device;
  extern ID3D11DeviceContext1* g_device_context;
  extern IDXGISwapChain1* g_swap_chain;
  extern ID3D11RenderTargetView* g_frame_buffer_view;

  bool create_device();
  bool create_debug_layer();
  bool create_swap_chain(HWND hwnd);
  void create_render_target();
  
  void cleanup_device();
  void cleanup_render_target();
  
  bool update_texture_buffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture);
  bool load_texture_from_buffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
  bool lod_texture_from_file(const char* filename, int& out_width, int& out_height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
}