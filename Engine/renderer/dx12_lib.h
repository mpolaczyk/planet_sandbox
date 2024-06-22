#pragma once

#include "d3d12.h"
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_core.h"
#include "d3dx12/d3dx12_barriers.h"

#include <dxgi1_6.h>

#include <vector>
#include <wrl/client.h>

#include "core/core.h"
#include "math/vertex_data.h"


struct ID3D12Device;
struct ID3D12RootSignature;
struct IDXGISwapChain3;
struct ID3D12RenderTargetView;
struct IDXGIFactory1;
struct IDXGIAdapter1;
struct ID3D12CommandQueue;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

struct ID3D12PipelineState;

#if !defined(_WINDEF_) && !defined(__INTELLISENSE__)
class HWND__;
typedef HWND__* HWND;
#endif

#define DX_RELEASE(resource) if(resource) { resource.Reset(); }

namespace engine
{
  using Microsoft::WRL::ComPtr;

  struct ENGINE_API fframe_context
  {
    ComPtr<ID3D12CommandAllocator>  command_allocator;
    UINT64                          fence_value;
  };
  
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
    //void create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, ComPtr<ID3D11Buffer>& out_vertex_buffer) const;
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
    
    void create_pipeline(HWND hwnd);
    
    void wait_for_gpu();
    void cleanup();
    

    static void get_hw_adapter(IDXGIFactory1* in_factory, IDXGIAdapter1** out_adapter, bool prefer_high_performance_adapter = false);
    static constexpr UINT back_buffer_count = 2;
    
    // Pipeline
    CD3DX12_VIEWPORT viewport;
    CD3DX12_RECT scissor_rect;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> command_queue;
    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<IDXGISwapChain3> swap_chain;
    ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
    UINT rtv_descriptor_size = 0;
    //D3D12_CPU_DESCRIPTOR_HANDLE rtv_descriptors[back_buffer_count];
    ComPtr<ID3D12DescriptorHeap> srv_descriptor_heap;
    ComPtr<ID3D12Resource> rtv[back_buffer_count];
    ComPtr<ID3D12CommandAllocator> command_allocators[back_buffer_count];
    ComPtr<ID3D12GraphicsCommandList> command_list;
    ComPtr<ID3D12PipelineState> pipeline_state;

    
    // Synchronization
    UINT64 back_buffer_index = 0;
    HANDLE fence_event = nullptr;
    ComPtr<ID3D12Fence> fence;
    UINT64 fence_values[back_buffer_count] = {};


    bool swap_chain_occluded = false;
  };
}
