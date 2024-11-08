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

#if !defined(_WINDEF_) && !defined(__INTELLISENSE__)
class HWND__;
typedef HWND__* HWND;
#endif

#define DX_RELEASE(resource) if(resource) { resource.Reset(); }

// std::basic_format_string<char>::basic_format_string': call to immediate function is not a constant expression
// This means that format string needs to be known at the compile time.
// Replace: DX_SET_NAME(resource, name)
// With: DX_SET_NAME(resource, "{}", name) 
#define DX_SET_NAME(resource, ...)  { std::string __local_name = std::format(__VA_ARGS__); \
    resource->SetName(std::wstring(__local_name.begin(), __local_name.end()).c_str()); }

namespace engine
{
  struct fdescriptor_heap;
  using Microsoft::WRL::ComPtr;
  
  struct fpipeline_state_stream;
  struct fstatic_mesh_render_state;
  struct ftexture_render_state;
  class atexture;
  
  class ENGINE_API fdx12 final
  {
  public:
    static DXGI_SAMPLE_DESC get_multisample_quality_levels(ComPtr<ID3D12Device> device, DXGI_FORMAT format, UINT num_samples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags);

    static void enable_debug_layer();
    static bool enable_screen_tearing(ComPtr<IDXGIFactory4> factory);
    static void enable_info_queue(ComPtr<ID3D12Device> device);
    
    static void create_factory(ComPtr<IDXGIFactory4>& out_factory4);
    
    static void create_swap_chain(HWND hwnd, IDXGIFactory4* factory, ID3D12CommandQueue* command_queue, uint32_t back_buffer_count, bool allow_screen_tearing, ComPtr<IDXGISwapChain4>& out_swap_chain);
    static void create_root_signature(ComPtr<ID3D12Device> device, const std::vector<CD3DX12_ROOT_PARAMETER1>& root_parameters, const std::vector<CD3DX12_STATIC_SAMPLER_DESC>& static_samplers, D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags, ComPtr<ID3D12RootSignature>& out_root_signature, const char* name);
    static void create_pipeline_state(ComPtr<ID3D12Device2> device, fpipeline_state_stream& pipeline_state_stream, ComPtr<ID3D12PipelineState>& out_pipeline_state, const char* name);
    static void create_pipeline_state(ComPtr<ID3D12Device2> device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc, ComPtr<ID3D12PipelineState>& out_pipeline_state, const char* name);
    static void create_texture_resource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource>& out_resource, ComPtr<ID3D12Resource>& out_upload_resource);


    // CLONE to the one in fdevice
    static void create_upload_resource(ComPtr<ID3D12Device> device, uint64_t buffer_size, ComPtr<ID3D12Resource>& out_resource);


    
    static void resize_swap_chain(ComPtr<IDXGISwapChain4> swap_chain, uint32_t backbuffer_count, uint32_t width, uint32_t height);
    
    
    static void report_live_objects();
    
    // TODO Check if they do the same in the end...
    static void upload_buffer(ComPtr<ID3D12Resource> resource, uint64_t buffer_size, const void* in_buffer);
    
    
    
    static bool get_dxc_hash(ComPtr<IDxcResult> result, std::string& out_hash);
    static bool get_dxc_blob(ComPtr<IDxcResult> result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlob>& out_blob);
    static bool get_dxc_blob(ComPtr<IDxcResult> result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlobUtf8>& out_blob);
    static bool save_dxc_blob(ComPtr<IDxcBlob> blob, const char* path);
    
    static uint64_t align_size_to(uint64_t size, uint64_t value)
    {
      return (size + value) & ~value;
    }
  };
}
