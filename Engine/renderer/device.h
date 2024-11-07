#pragma once

#include <wrl/client.h>
#include <vector>
#include <string>

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
struct CD3DX12_ROOT_PARAMETER1;
struct CD3DX12_STATIC_SAMPLER_DESC;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct DXGI_SAMPLE_DESC;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct IDxcResult;
struct IDxcBlobUtf8;
struct IDxcBlob;
struct IDxcIncludeHandler;
struct IDxcCompiler3;
struct IDxcUtils;
enum D3D12_RESOURCE_STATES;
enum D3D12_RESOURCE_FLAGS;
enum D3D12_ROOT_SIGNATURE_FLAGS;
enum DXGI_FORMAT;
enum D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS;
enum DXC_OUT_KIND;

namespace engine
{
  class atexture;
  struct ftexture_resource;
  struct fshader_resource_buffer;
  struct fconst_buffer;
  struct fdescriptor_heap;
  
  using namespace Microsoft::WRL;
  
  struct ENGINE_API fdevice
  {
    static void get_hw_adapter(IDXGIFactory1* factory, IDXGIAdapter1** out_adapter, bool prefer_high_performance_adapter = false);
    static fdevice create(IDXGIFactory4* factory);
    
    void create_render_target(IDXGISwapChain4* swap_chain, ID3D12DescriptorHeap* descriptor_heap, uint32_t back_buffer_count, std::vector<ComPtr<ID3D12Resource>>& out_rtv);
    void create_depth_stencil(ID3D12DescriptorHeap* descriptor_heap, uint32_t width, uint32_t height, ComPtr<ID3D12Resource>& out_dsv);
    void create_command_queue(ComPtr<ID3D12CommandQueue>& out_command_queue);
    void create_command_list(uint32_t back_buffer_count, ComPtr<ID3D12GraphicsCommandList>& out_command_list, std::vector<ComPtr<ID3D12CommandAllocator>>& out_command_allocators);
    void create_synchronisation(uint32_t back_buffer_count, uint64_t initial_fence_value, ComPtr<ID3D12Fence>& out_fence, HANDLE& out_fence_event, std::vector<uint64_t>& out_fence_values);

    void create_render_target_descriptor_heap(uint32_t back_buffer_count, ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap);
    void create_depth_stencil_descriptor_heap(ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap);
    void create_cbv_srv_uav_descriptor_heap(fdescriptor_heap& out_descriptor_heap);
    
    void create_const_buffer(fdescriptor_heap* heap, uint64_t in_size, fconst_buffer& out_buffer, const char* name);
    void create_shader_resource_buffer(fdescriptor_heap* heap, uint64_t in_size, fshader_resource_buffer& out_buffer, const char* name);
    void create_texture_resource(fdescriptor_heap* heap, atexture* texture_asset, const char* name);

    void create_upload_resource(uint64_t buffer_size, ComPtr<ID3D12Resource>& out_resource);
    void create_buffer_resource(uint64_t buffer_size, ComPtr<ID3D12Resource>& out_resource);
    
    ComPtr<ID3D12Device2> device;
  };
}
