#pragma once

#include <string>

#include "core/core.h"
#include "core/com_pointer.h"

struct IDXGISwapChain4;
struct IDXGIFactory4;
struct ID3D12CommandQueue;
struct ID3D12Resource;
struct IDxcResult;
struct IDxcBlobUtf8;
struct IDxcBlob;
enum DXC_OUT_KIND;
enum DXGI_FORMAT;

#if !defined(_WINDEF_) && !defined(__INTELLISENSE__)
struct HWND__;
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
  // Container for all code that does not fit one of the classes: fdevice, fcommand_list or fgraphics_pipeline etc.
  struct ENGINE_API fdx12
  {
    static void enable_debug_layer_and_gpu_validation();
    
    static bool enable_screen_tearing(ComPtr<IDXGIFactory4> factory);
    static void create_factory(ComPtr<IDXGIFactory4>& out_factory4);
    static void create_swap_chain(HWND hwnd, IDXGIFactory4* factory, ID3D12CommandQueue* command_queue, uint32_t back_buffer_count, DXGI_FORMAT format, bool allow_screen_tearing, ComPtr<IDXGISwapChain4>& out_swap_chain);
    
    static void resize_swap_chain(IDXGISwapChain4* swap_chain, uint32_t backbuffer_count, uint32_t width, uint32_t height);
    
    static void report_live_objects();
    
    static void upload_host_buffer(ID3D12Resource* resource, uint32_t buffer_size, const void* in_buffer);
    
    static bool get_dxc_hash(IDxcResult* result, std::string& out_hash);
    static bool get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlob>& out_blob);
    static bool get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlobUtf8>& out_blob);
    static bool save_dxc_blob(IDxcBlob* blob, const char* path);

    static std::string get_resource_name(ID3D12Resource* resource);
    
    static uint32_t align_size_to(uint32_t size, uint32_t value)
    {
      return (size + value) & ~value;
    }
  };
}
