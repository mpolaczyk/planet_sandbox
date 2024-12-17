
#include <winerror.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <fstream>

#include "d3d12.h"
#include "d3dx12/d3dx12_core.h"

#include "engine/renderer/dx12_lib.h"

#include "core/exceptions/windows_error.h"
#include "engine/log.h"
#include "engine/string_tools.h"

namespace engine
{
  void fdx12::enable_debug_layer_and_gpu_validation()
  {
    ComPtr<ID3D12Debug> debug;
    ComPtr<ID3D12Debug1> debug1;
    THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())))
    THROW_IF_FAILED(debug->QueryInterface(IID_PPV_ARGS(debug1.GetAddressOf())))
    debug1->SetEnableGPUBasedValidation(true);
    debug->EnableDebugLayer();
    LOG_DEBUG("Enabled debug layer and GPU validation")
  }

  bool fdx12::enable_screen_tearing(ComPtr<IDXGIFactory4> factory)
  {
    uint32_t success = 1;
    ComPtr<IDXGIFactory5> factory5;
    if(SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(factory5.GetAddressOf()))))
    {
      success = FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &success, sizeof(success))) ? 0 : 1;
    }
    return success == 1;
  }
  
  void fdx12::create_factory(ComPtr<IDXGIFactory4>& out_factory4)
  {
    uint32_t factory_flags = 0;
#if BUILD_DEBUG && !USE_NSIGHT_AFTERMATH
    factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    THROW_IF_FAILED(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(out_factory4.GetAddressOf())))
  }
  
  void fdx12::create_swap_chain(HWND hwnd, IDXGIFactory4* factory, ID3D12CommandQueue* command_queue, uint32_t back_buffer_count, DXGI_FORMAT format, bool allow_screen_tearing, ComPtr<IDXGISwapChain4>& out_swap_chain)
  {
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.BufferCount = back_buffer_count;
    desc.Flags = allow_screen_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    desc.Width = 0;
    desc.Height = 0;
    desc.Format = format;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Stereo = FALSE;

    ComPtr<IDXGISwapChain1> swap_chain1;
    THROW_IF_FAILED(factory->CreateSwapChainForHwnd(command_queue, hwnd, &desc, nullptr, nullptr, &swap_chain1))
    THROW_IF_FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER))
    THROW_IF_FAILED(swap_chain1->QueryInterface(IID_PPV_ARGS(out_swap_chain.GetAddressOf())))
  }
  
  void fdx12::resize_swap_chain(IDXGISwapChain4* swap_chain, uint32_t backbuffer_count, uint32_t width, uint32_t height)
  {
    DXGI_SWAP_CHAIN_DESC desc = {};
    THROW_IF_FAILED(swap_chain->GetDesc(&desc))
    THROW_IF_FAILED(swap_chain->ResizeBuffers(backbuffer_count, width, height, desc.BufferDesc.Format, desc.Flags))
  }
  
  void fdx12::report_live_objects()
  {
    ComPtr<IDXGIDebug1> debug;
    THROW_IF_FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(debug.GetAddressOf())))
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
  }

  void fdx12::upload_host_buffer(ID3D12Resource* resource, uint32_t buffer_size, const void* in_buffer)
  {
    CD3DX12_RANGE read_range(0, 0);
    uint8_t* mapping = nullptr;
    THROW_IF_FAILED(resource->Map(0, &read_range, reinterpret_cast<void**>(&mapping)))
    memcpy(mapping, in_buffer, buffer_size);
    resource->Unmap(0, nullptr);
  }

  std::string fdx12::get_resource_name(ID3D12Resource* resource)
  {
    wchar_t name[128] = {};
    UINT size = sizeof(name);
    resource->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name);
    return fstring_tools::to_utf8(name);
  }
}