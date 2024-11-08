
#include <winerror.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <fstream>

#include "d3d12.h"
#include "d3dx12/d3dx12_core.h"
#include "dxcapi.h"

#include "renderer/dx12_lib.h"

#include "core/exceptions.h"
#include "engine/log.h"
#include "engine/string_tools.h"

namespace engine
{
  void fdx12::enable_debug_layer()
  {
    ComPtr<ID3D12Debug> debug;
    ComPtr<ID3D12Debug1> debug1;
    THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())))
    THROW_IF_FAILED(debug->QueryInterface(IID_PPV_ARGS(debug1.GetAddressOf())))
    debug1->SetEnableGPUBasedValidation(true);
    debug->EnableDebugLayer();
  }

  bool fdx12::enable_screen_tearing(ComPtr<IDXGIFactory4> factory)
  {
    bool success = false;
    ComPtr<IDXGIFactory5> factory5;
    if(SUCCEEDED(factory.As(&factory5)))
    {
      if(FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &success, sizeof(success))))
      {
        success = false;
      }
    }
    return success;
  }
  
  void fdx12::create_factory(ComPtr<IDXGIFactory4>& out_factory4)
  {
    uint32_t factory_flags = 0;
#if BUILD_DEBUG && !USE_NSIGHT_AFTERMATH
    factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    THROW_IF_FAILED(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(out_factory4.GetAddressOf())))
  }
  
  void fdx12::create_swap_chain(HWND hwnd, IDXGIFactory4* factory, ID3D12CommandQueue* command_queue, uint32_t back_buffer_count, bool allow_screen_tearing, ComPtr<IDXGISwapChain4>& out_swap_chain)
  {
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.BufferCount = back_buffer_count;
    desc.Flags = allow_screen_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    desc.Width = 0;
    desc.Height = 0;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Stereo = FALSE;

    ComPtr<IDXGISwapChain1> swap_chain1;
    THROW_IF_FAILED(factory->CreateSwapChainForHwnd(command_queue, hwnd, &desc, nullptr, nullptr, &swap_chain1))
    THROW_IF_FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    THROW_IF_FAILED(swap_chain1.As(&out_swap_chain))
  }
  
  void fdx12::resize_swap_chain(IDXGISwapChain4* swap_chain, uint32_t back_buffer_count, uint32_t width, uint32_t height)
  {
    DXGI_SWAP_CHAIN_DESC desc = {};
    THROW_IF_FAILED(swap_chain->GetDesc(&desc))
    THROW_IF_FAILED(swap_chain->ResizeBuffers(back_buffer_count, width, height, desc.BufferDesc.Format, desc.Flags))
  }
  
  void fdx12::report_live_objects()
  {
    ComPtr<IDXGIDebug1> debug;
    THROW_IF_FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(debug.GetAddressOf())))
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
  }

  void fdx12::upload_host_buffer(ID3D12Resource* resource, uint64_t buffer_size, const void* in_buffer)
  {
    CD3DX12_RANGE read_range(0, 0);
    uint8_t* mapping = nullptr;
    THROW_IF_FAILED(resource->Map(0, &read_range, reinterpret_cast<void**>(&mapping)));
    memcpy(mapping, in_buffer, buffer_size);
    resource->Unmap(0, nullptr);
  }

  bool fdx12::get_dxc_hash(IDxcResult* result, std::string& out_hash)
  {
    ComPtr<IDxcBlob> hash = nullptr;
    char hash_string[32] = {'\0'};
    if(fdx12::get_dxc_blob(result, DXC_OUT_SHADER_HASH, hash) && hash != nullptr)
    {
      auto* hash_buffer = static_cast<DxcShaderHash*>(hash->GetBufferPointer());
      for(size_t i = 0; i < _countof(hash_buffer->HashDigest); ++i)
      {
        snprintf(hash_string + i, 16, "%X", hash_buffer->HashDigest[i]);
      }
      out_hash = std::string(hash_string);
      return true;
    }
    return false;
  }
  
  bool fdx12::get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlob>& out_blob)
  {
    ComPtr<IDxcBlobUtf16> name = nullptr;
    if(FAILED(result->GetOutput(blob_type, IID_PPV_ARGS(out_blob.GetAddressOf()), name.GetAddressOf())) && out_blob != nullptr)
    {
      LOG_ERROR("Unable to get dxc blob {0}", static_cast<int32_t>(blob_type));
      return false;
    }
    return true;
  }
  
  bool fdx12::get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlobUtf8>& out_blob)
  {
    ComPtr<IDxcBlobUtf16> name = nullptr;
    if(FAILED(result->GetOutput(blob_type, IID_PPV_ARGS(out_blob.GetAddressOf()), name.GetAddressOf())) && out_blob != nullptr)
    {
      LOG_ERROR("Unable to get dxc blob {0}", static_cast<int32_t>(blob_type));
      return false;
    }
    return true;
  } 
  
  bool fdx12::save_dxc_blob(IDxcBlob* blob, const char* path)
  {
    FILE* file = nullptr;
    std::wstring w_path = fstring_tools::to_utf16(path);
    errno_t open_result = _wfopen_s(&file, w_path.c_str(), L"wb");
    if(open_result != 0)
    {
      // See https://learn.microsoft.com/en-us/cpp/c-runtime-library/errno-constants
      LOG_ERROR("Failed to write dxc blob. Error code {0}", static_cast<int32_t>(open_result));
      return false;
    }
    fwrite(blob->GetBufferPointer(), blob->GetBufferSize(), 1, file);
    fclose(file);
    return true;
  }
}