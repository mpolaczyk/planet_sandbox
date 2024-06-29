#pragma once

#include <vector>
#include <wrl/client.h>

#include "core/core.h"
#include "math/vertex_data.h"
#include "renderer/render_state.h"

struct ID3D12Device;
struct ID3D12RootSignature;
struct IDXGISwapChain3;
struct ID3D12RenderTargetView;
struct IDXGIFactory1;
struct IDXGIFactory4;
struct IDXGIAdapter1;
struct ID3D12CommandQueue;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

#if !defined(_WINDEF_) && !defined(__INTELLISENSE__)
class HWND__;
typedef HWND__* HWND;
#endif

#define DX_RELEASE(resource) if(resource) { resource.Reset(); }

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  class ENGINE_API fdx12 final
  {
  public:
    static fdx12& instance()
    {
      static fdx12 singleton;
      return singleton;
    }
    
    //void create_input_layout(const D3D11_INPUT_ELEMENT_DESC* input_element_desc, uint32_t input_element_desc_size, const ComPtr<ID3D10Blob>& vertex_shader_blob, ComPtr<ID3D11InputLayout>& input_layout) const;
    //void create_sampler_state(ComPtr<ID3D11SamplerState>& out_sampler_state) const;
    //void create_rasterizer_state(ComPtr<ID3D11RasterizerState>& out_rasterizer_state) const;
    //void create_depth_stencil_state(ComPtr<ID3D11DepthStencilState>& out_depth_stencil_state) const;
    //
    //void create_constant_buffer(uint32_t size, ComPtr<ID3D11Buffer>& out_constant_buffer) const;
    //void create_index_buffer(const std::vector<fface_data>& in_face_list, ComPtr<ID3D11Buffer>& out_index_buffer) const;
    
    //
    //void create_texture(uint32_t width, uint32_t height, DXGI_FORMAT format, D3D11_BIND_FLAG bind_flags, D3D11_USAGE usage, ComPtr<ID3D11Texture2D>& out_texture, uint32_t bytes_per_row = 0, const void* in_bytes = nullptr) const;
    //
    //void create_render_target_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_RTV_DIMENSION view_dimmension, ComPtr<ID3D11RenderTargetView>& out_render_target_view) const;
    //void create_depth_stencil_view(const ComPtr<ID3D11Texture2D>& in_texture, uint32_t width, uint32_t height, ComPtr<ID3D11DepthStencilView>& out_depth_stencil_view) const;
    //void create_shader_resource_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_SRV_DIMENSION view_dimmension, ComPtr<ID3D11ShaderResourceView>& out_shader_resource_view) const;
    //
    //void create_vertex_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11VertexShader>& out_vertex_shader) const;
    //void create_pixel_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11PixelShader>& out_pixel_shader) const;
    //
    //template <typename T>
    //void update_constant_buffer(T* data, ComPtr<ID3D11Buffer>& out_constant_buffer) const
    //{
    //  D3D11_MAPPED_SUBRESOURCE mapped_subresource_data;
    //  device_context->Map(out_constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource_data);
    //  *static_cast<T*>(mapped_subresource_data.pData) = *data;
    //  device_context->Unmap(out_constant_buffer.Get(), 0);
    //}
    
    static void get_hw_adapter(IDXGIFactory1* in_factory, IDXGIAdapter1** out_adapter, bool prefer_high_performance_adapter = false);
    static void enable_debug_layer();
    static void create_factory(ComPtr<IDXGIFactory4>& out_factory4);
    static void create_device(const ComPtr<IDXGIFactory4>& in_factory, ComPtr<ID3D12Device>& out_device);
    static bool enable_screen_tearing(const ComPtr<IDXGIFactory4>& in_factory);
    static void enable_info_queue(const ComPtr<ID3D12Device>& in_device);
    static void create_command_queue(const ComPtr<ID3D12Device>& in_device, ComPtr<ID3D12CommandQueue>& out_command_queue);
    static void create_swap_chain(HWND hwnd, const ComPtr<IDXGIFactory4>& in_factory, const ComPtr<ID3D12CommandQueue>& in_command_queue, int back_buffer_count, bool allow_screen_tearing, ComPtr<IDXGISwapChain3>& out_swap_chain);
    static void create_render_target(const ComPtr<ID3D12Device>& in_device, const ComPtr<IDXGISwapChain3>& in_swap_chain, int back_buffer_count, ComPtr<ID3D12DescriptorHeap>& out_rtv_descriptor_heap, uint32_t& out_rtv_descriptor_size, std::vector<ComPtr<ID3D12Resource>>& out_resource);
    static void create_shader_resource(const ComPtr<ID3D12Device>& in_device, ComPtr<ID3D12DescriptorHeap>& out_srv_descriptor_heap);
    static void create_command_list(const ComPtr<ID3D12Device>& in_device, int back_buffer_count, ComPtr<ID3D12GraphicsCommandList>& out_command_list, std::vector<ComPtr<ID3D12CommandAllocator>>& out_command_allocators);
    static void create_root_signature(const ComPtr<ID3D12Device>& in_device, ComPtr<ID3D12RootSignature>& out_root_signature);
    static void create_synchronisation(const ComPtr<ID3D12Device>& in_device, int back_buffer_count, int initial_fence_value, ComPtr<ID3D12Fence>& out_fence, HANDLE& out_fence_event, std::vector<uint64_t>& out_fence_values);
    static void report_live_objects();
    
    void create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, fstatic_mesh_render_state& out_render_state);
  };
}
