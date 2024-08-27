#pragma once

#include <wrl/client.h>
#include <vector>

#include "core/core.h"

struct ID3D12Device;
struct ID3D12Device2;
struct ID3D12RootSignature;
struct IDXGISwapChain4;
struct ID3D12RenderTargetView;
struct IDXGIFactory1;
struct IDXGIFactory4;
struct IDXGIAdapter1;
struct ID3D12Fence;
struct ID3D12CommandQueue;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12PipelineState;
struct D3D12_ROOT_PARAMETER;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
enum D3D12_RESOURCE_STATES;
enum D3D12_ROOT_SIGNATURE_FLAGS;

#if !defined(_WINDEF_) && !defined(__INTELLISENSE__)
class HWND__;
typedef HWND__* HWND;
#endif

#define DX_RELEASE(resource) if(resource) { resource.Reset(); }

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fpipeline_state_stream;
  struct fstatic_mesh_render_state;
  
  class ENGINE_API fdx12 final
  {
  public:
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
    //void create_vertex_shader(const ComPtr<ID3D10Blob>& blob, ComPtr<ID3D11VertexShader>& out_vertex_shader) const;
    //void create_pixel_shader(const ComPtr<ID3D10Blob>& blob, ComPtr<ID3D11PixelShader>& out_pixel_shader) const;
    //
    //template <typename T>
    //void update_constant_buffer(T* data, ComPtr<ID3D11Buffer>& out_constant_buffer) const
    //{
    //  D3D11_MAPPED_SUBRESOURCE mapped_subresource_data;
    //  device_context->Map(out_constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource_data);
    //  *static_cast<T*>(mapped_subresource_data.pData) = *data;
    //  device_context->Unmap(out_constant_buffer.Get(), 0);
    //}
    
    static void get_hw_adapter(IDXGIFactory1* factory, IDXGIAdapter1** out_adapter, bool prefer_high_performance_adapter = false);
    
    static void enable_debug_layer();
    static bool enable_screen_tearing(ComPtr<IDXGIFactory4> factory);
    static void enable_info_queue(ComPtr<ID3D12Device> device);
    
    static void create_factory(ComPtr<IDXGIFactory4>& out_factory4);
    static void create_device(ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12Device2>& out_device);
    static void create_command_queue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& out_command_queue);
    static void create_swap_chain(HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue, uint32_t back_buffer_count, bool allow_screen_tearing, ComPtr<IDXGISwapChain4>& out_swap_chain);
    static void create_render_target_descriptor_heap(ComPtr<ID3D12Device> device, uint32_t back_buffer_count, ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap);
    static void create_depth_stencil_descriptor_heap(ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap);
    static void create_render_target(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swap_chain, ComPtr<ID3D12DescriptorHeap> descriptor_heap, uint32_t back_buffer_count, std::vector<ComPtr<ID3D12Resource>>& out_rtv);
    static void create_depth_stencil(ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap> descriptor_heap, int width, int height, ComPtr<ID3D12Resource>& out_dsv);
    static void create_main_descriptor_heap(ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap>& out_main_descriptor_heap);
    static void create_command_list(ComPtr<ID3D12Device> device, uint32_t back_buffer_count, ComPtr<ID3D12GraphicsCommandList>& out_command_list, std::vector<ComPtr<ID3D12CommandAllocator>>& out_command_allocators);
    static void create_synchronisation(ComPtr<ID3D12Device> device, uint32_t back_buffer_count, uint64_t initial_fence_value, ComPtr<ID3D12Fence>& out_fence, HANDLE& out_fence_event, std::vector<uint64_t>& out_fence_values);
    static void create_root_signature(ComPtr<ID3D12Device> device, const std::vector<D3D12_ROOT_PARAMETER>& root_parameters,  D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags, ComPtr<ID3D12RootSignature>& out_root_signature);
    static void create_pipeline_state(ComPtr<ID3D12Device2> device, fpipeline_state_stream& pipeline_state_stream, ComPtr<ID3D12PipelineState>& out_pipeline_state);
    static void create_upload_resource(ComPtr<ID3D12Device> device, size_t buffer_size, ComPtr<ID3D12Resource>& out_resource);
    static void create_default_resource(ComPtr<ID3D12Device> device, size_t buffer_size, ComPtr<ID3D12Resource>& out_resource);

    static void resize_swap_chain(ComPtr<IDXGISwapChain4> swap_chain, uint32_t backbuffer_count, uint32_t width, uint32_t height);
    
    static void set_render_targets(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap, ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap, int back_buffer_index);
    static void set_viewport(ComPtr<ID3D12GraphicsCommandList> command_list, uint32_t width, uint32_t height);
    static void set_scissor(ComPtr<ID3D12GraphicsCommandList> command_list, uint32_t width, uint32_t height);

    static void clear_render_target(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12DescriptorHeap> descriptor_heap, uint32_t back_buffer_index);
    static void clear_depth_stencil(ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12DescriptorHeap> descriptor_heap);
    
    static void report_live_objects();
    
    static void resource_barrier(ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after);
    static void upload_buffer_resource(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, size_t buffer_size, const void* in_buffer, ComPtr<ID3D12Resource>& out_upload_intermediate, ComPtr<ID3D12Resource>& out_gpu_resource);
    static void upload_vertex_buffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, fstatic_mesh_render_state& out_render_state);
    static void upload_index_buffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, fstatic_mesh_render_state& out_render_state);
    
  };
}
