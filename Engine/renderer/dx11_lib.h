#pragma once

#include "core/core.h"

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

#if !defined(_WINDEF_) && !defined(__INTELLISENSE__)
class HWND__;
typedef HWND__* HWND;
#endif

#define DX_RELEASE(resource) if(resource!=nullptr) { resource->Release(); resource = nullptr; }

namespace engine
{
  class ENGINE_API dx11 final
  {
  public:
    static dx11& instance()
    {
      static dx11 singleton;
      return singleton;
    }

    // Main dx11 objects
    ID3D11Device1* device = nullptr;
    ID3D11DeviceContext1* device_context = nullptr;
    IDXGISwapChain1* swap_chain = nullptr;

    // Window rtv
    ID3D11RenderTargetView* rtv = nullptr;
    
    bool create_device();
    bool create_debug_layer();
    bool create_swap_chain(HWND hwnd);
    void create_render_target();
    
    void cleanup_device();
    void cleanup_render_target();

    bool update_texture_buffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture);
    bool load_texture_from_buffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
  };
}