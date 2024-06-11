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

    // TODO: Most of create_... methods can be made static, it's jut helper code.
    // HELPER FUNCTIONS
    
    void create_input_layout(const D3D11_INPUT_ELEMENT_DESC* input_element_desc, uint32_t input_element_desc_size, const ComPtr<ID3D10Blob>& vertex_shader_blob, ComPtr<ID3D11InputLayout>& input_layout) const;
    void create_sampler_state(ComPtr<ID3D11SamplerState>& out_sampler_state) const;
    void create_rasterizer_state(ComPtr<ID3D11RasterizerState>& out_rasterizer_state) const;
    void create_depth_stencil_state(ComPtr<ID3D11DepthStencilState>& out_depth_stencil_state) const;

    void create_constant_buffer(uint32_t size, ComPtr<ID3D11Buffer>& out_constant_buffer) const;
    void create_index_buffer(const std::vector<fface_data>& in_face_list, ComPtr<ID3D11Buffer>& out_index_buffer) const;
    void create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, ComPtr<ID3D11Buffer>& out_vertex_buffer) const;

    void create_texture(uint32_t width, uint32_t height, DXGI_FORMAT format, D3D11_BIND_FLAG bind_flags, D3D11_USAGE usage, ComPtr<ID3D11Texture2D>& out_texture, uint32_t bytes_per_row = 0, const void* in_bytes = nullptr) const;
    
    void create_render_target_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_RTV_DIMENSION view_dimmension, ComPtr<ID3D11RenderTargetView>& out_render_target_view) const;
    void create_depth_stencil_view(const ComPtr<ID3D11Texture2D>& in_texture, uint32_t width, uint32_t height, ComPtr<ID3D11DepthStencilView>& out_depth_stencil_view) const;
    void create_shader_resource_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_SRV_DIMENSION view_dimmension, ComPtr<ID3D11ShaderResourceView>& out_shader_resource_view) const;

    void create_vertex_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11VertexShader>& out_vertex_shader) const;
    void create_pixel_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11PixelShader>& out_pixel_shader) const;

    template <typename T>
    void update_constant_buffer(T* data, ComPtr<ID3D11Buffer>& out_constant_buffer) const
    {
      D3D11_MAPPED_SUBRESOURCE mapped_subresource_data;
      device_context->Map(out_constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource_data);
      *static_cast<T*>(mapped_subresource_data.pData) = *data;
      device_context->Unmap(out_constant_buffer.Get(), 0);
    }

    // INSTANCE FUNCTIONS AND STATE

    void create_device();
    void create_debug_layer() const;
    void create_swap_chain(HWND hwnd);
    void create_render_target();
    
    void cleanup_device();
    void cleanup_render_target();

    // Main dx11 objects
    ComPtr<ID3D11Device1> device;
    ComPtr<ID3D11DeviceContext1> device_context;
    ComPtr<IDXGISwapChain1> swap_chain;

    // Window rtv
    ComPtr<ID3D11RenderTargetView> rtv;
  };
}
