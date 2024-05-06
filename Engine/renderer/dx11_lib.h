#pragma once

#include <vector>
#include <wrl/client.h>

#include "core/core.h"
#include "math/vertex_data.h"

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11Buffer;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D10Blob;
struct ID3D11InputLayout;

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

    void create_input_layout(const D3D11_INPUT_ELEMENT_DESC* input_element_desc, uint32_t input_element_desc_size, ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11InputLayout>& input_layout) const;
    void create_sampler_state(ComPtr<ID3D11SamplerState>& out_sampler_state) const;
    void create_constant_buffer(uint32_t size, ComPtr<ID3D11Buffer>& out_constant_buffer) const;
    void create_rasterizer_state(ComPtr<ID3D11RasterizerState>& out_rasterizer_state) const;
    void create_depth_stencil_state(ComPtr<ID3D11DepthStencilState>& out_depth_stencil_state) const;
        
    void cleanup_device();
    void cleanup_render_target();

    static bool create_index_buffer(const std::vector<fface_data>& in_face_list, ComPtr<ID3D11Buffer>& out_index_buffer);
    static bool create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, ComPtr<ID3D11Buffer>& out_vertex_buffer);

    static bool create_texture_from_buffer(unsigned char* in_buffer, int width, int height, ComPtr<ID3D11ShaderResourceView>& out_srv, ComPtr<ID3D11Texture2D>& out_texture);
    static bool update_texture_from_buffer(unsigned char* in_buffer, int width, int height, ComPtr<ID3D11Texture2D>& out_texture);
  };
}