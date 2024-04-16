#pragma once

#include <wrl/client.h>

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

#define DX_RELEASE(resource) if(resource) { resource.Reset(); }

namespace engine
{
  using Microsoft::WRL::ComPtr;

  class ENGINE_API fdx11 final
  {
  public:
    static fdx11& instance()
    {
      static fdx11 singleton;
      return singleton;
    }

    // Main dx11 objects
    ComPtr<ID3D11Device1> device;
    ComPtr<ID3D11DeviceContext1> device_context;
    ComPtr<IDXGISwapChain1> swap_chain;

    // Window rtv
    ComPtr<ID3D11RenderTargetView> rtv;
    
    void create_device();
    void create_debug_layer() const;
    void create_swap_chain(HWND hwnd);
    void create_render_target();
    
    void cleanup_device();
    void cleanup_render_target();

    bool load_texture_from_buffer(unsigned char* buffer, int width, int height, ComPtr<ID3D11ShaderResourceView>& out_srv, ComPtr<ID3D11Texture2D>& out_texture) const;
    bool update_texture_buffer(unsigned char* buffer, int width, int height, const ComPtr<ID3D11Texture2D>& in_texture) const;
  };
}