#include <winerror.h>

#include "renderer/dx12_lib.h"

#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <DirectXColors.h>
#include <fstream>

#include "d3d12.h"
#include <sstream>

#include "dxcapi.h"
#include "assets/texture.h"
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_barriers.h"
#include "d3dx12/d3dx12_resource_helpers.h"

#include "core/exceptions.h"
#include "engine/io.h"
#include "engine/log.h"
#include "engine/string_tools.h"
#include "renderer/aligned_structs.h"
#include "renderer/descriptor_heap.h"
#include "renderer/pipeline_state.h"
#include "renderer/render_state.h"

namespace engine
{
  DXGI_SAMPLE_DESC fdx12::get_multisample_quality_levels(ComPtr<ID3D12Device> device, DXGI_FORMAT format, UINT num_samples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags)
  {
    DXGI_SAMPLE_DESC desc = { 1, 0 };

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS quality_levels;
    quality_levels.Format           = format;
    quality_levels.SampleCount      = 1;
    quality_levels.Flags            = flags;
    quality_levels.NumQualityLevels = 0;

    while (quality_levels.SampleCount <= num_samples
        && SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &quality_levels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)))
        && quality_levels.NumQualityLevels > 0)
    {
      desc.Count   = quality_levels.SampleCount;
      desc.Quality = quality_levels.NumQualityLevels - 1;
      quality_levels.SampleCount *= 2;
    }
    return desc;
  }

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

  void fdx12::enable_info_queue(ComPtr<ID3D12Device> device)
  {
    ComPtr<ID3D12InfoQueue> info_queue;
    device->QueryInterface(IID_PPV_ARGS(info_queue.GetAddressOf()));
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
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
  
  void fdx12::resize_swap_chain(ComPtr<IDXGISwapChain4> swap_chain, uint32_t back_buffer_count, uint32_t width, uint32_t height)
  {
    DXGI_SWAP_CHAIN_DESC desc = {};
    THROW_IF_FAILED(swap_chain->GetDesc(&desc));
    THROW_IF_FAILED(swap_chain->ResizeBuffers(back_buffer_count, width, height, desc.BufferDesc.Format, desc.Flags));
  }
  
  void fdx12::create_root_signature(ComPtr<ID3D12Device> device, const std::vector<CD3DX12_ROOT_PARAMETER1>& root_parameters, const std::vector<CD3DX12_STATIC_SAMPLER_DESC>& static_samplers, D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags, ComPtr<ID3D12RootSignature>& out_root_signature)
  {
    D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data = {};
    feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(feature_data))))
    {
      feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
      
    // Serialize the root signature.
    ComPtr<ID3DBlob> root_signature_serialized;
    ComPtr<ID3DBlob> error_blob;
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc;
    root_signature_desc.Init_1_1(static_cast<uint32_t>(root_parameters.size()), root_parameters.data(), static_cast<uint32_t>(static_samplers.size()), static_samplers.data(), root_signature_flags);
    
    HRESULT hr;
    hr = D3DX12SerializeVersionedRootSignature(&root_signature_desc, feature_data.HighestVersion, root_signature_serialized.GetAddressOf(), error_blob.GetAddressOf());
    bool do_throw = false;
    std::ostringstream oss;
    if(FAILED(hr))
    {
      oss << "Root signature serialization failed. ";
      do_throw = true;
    }
    if (error_blob)
    {
      oss << static_cast<const char*>(error_blob->GetBufferPointer());
      do_throw = true;
    }
    if(do_throw)
    {
      throw fhresult_exception(hr, oss.str());
    }
    THROW_IF_FAILED(device->CreateRootSignature(0, root_signature_serialized->GetBufferPointer(), root_signature_serialized->GetBufferSize(), IID_PPV_ARGS(out_root_signature.GetAddressOf())));
  }

  void fdx12::create_pipeline_state(ComPtr<ID3D12Device2> device, fpipeline_state_stream& pipeline_state_stream, ComPtr<ID3D12PipelineState>& out_pipeline_state)
  {
    const D3D12_PIPELINE_STATE_STREAM_DESC pipeline_state_stream_desc(sizeof(fpipeline_state_stream), &pipeline_state_stream);
    THROW_IF_FAILED(device->CreatePipelineState(&pipeline_state_stream_desc, IID_PPV_ARGS(out_pipeline_state.GetAddressOf())));
  }

  void fdx12::create_pipeline_state(ComPtr<ID3D12Device2> device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc, ComPtr<ID3D12PipelineState>& out_pipeline_state)
  {
    THROW_IF_FAILED(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(out_pipeline_state.GetAddressOf())));
  }


  
  void fdx12::report_live_objects()
  {
    ComPtr<IDXGIDebug1> debug;
    THROW_IF_FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(debug.GetAddressOf())))
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
  }

  

  

  

  void fdx12::create_texture_resource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource>& out_resource, ComPtr<ID3D12Resource>& out_upload_resource)
  {
    D3D12_RESOURCE_DESC texture_desc = {};
    texture_desc.MipLevels = 1;
    texture_desc.Format = format;
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    texture_desc.DepthOrArraySize = 1;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    CD3DX12_HEAP_PROPERTIES default_heap(D3D12_HEAP_TYPE_DEFAULT);
    THROW_IF_FAILED(device->CreateCommittedResource(
      &default_heap,
      D3D12_HEAP_FLAG_NONE,
      &texture_desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&out_resource)));

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Format = texture_desc.Format;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(out_resource.Get(), &srv_desc, handle);
    
    const uint64_t buffer_size = GetRequiredIntermediateSize(out_resource.Get(), 0, 1);
    create_upload_resource(device, buffer_size, out_upload_resource);
  }
  
  void fdx12::create_upload_resource(ComPtr<ID3D12Device> device, uint64_t buffer_size, ComPtr<ID3D12Resource>& out_resource)
  {
    const CD3DX12_HEAP_PROPERTIES type_upload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size);
    THROW_IF_FAILED(device->CreateCommittedResource(
      &type_upload,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(out_resource.GetAddressOf())));
  }

  void fdx12::upload_buffer(ComPtr<ID3D12Resource> resource, uint64_t buffer_size, const void* in_buffer)
  {
    CD3DX12_RANGE read_range(0, 0);
    uint8_t* mapping = nullptr;
    THROW_IF_FAILED(resource->Map(0, &read_range, reinterpret_cast<void**>(&mapping)));
    memcpy(mapping, in_buffer, buffer_size);
    resource->Unmap(0, nullptr);
  }

  
  
  //void fdx12::upload_texture_buffer2(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, const CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, atexture* texture)
  //{
  //  ftexture_render_state& trs = texture->render_state;
  //  
  //  // Describe and create a Texture2D.
  //  D3D12_RESOURCE_DESC texture_desc = {};
  //  texture_desc.MipLevels = 1;
  //  texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // TODO move to texture, make persistent
  //  texture_desc.Width = texture->width;
  //  texture_desc.Height = texture->height;
  //  texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
  //  texture_desc.DepthOrArraySize = 1;
  //  texture_desc.SampleDesc.Count = 1;
  //  texture_desc.SampleDesc.Quality = 0;
  //  texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  //  
  //  CD3DX12_HEAP_PROPERTIES default_heap(D3D12_HEAP_TYPE_DEFAULT);
  //  THROW_IF_FAILED(device->CreateCommittedResource(
  //    &default_heap,
  //    D3D12_HEAP_FLAG_NONE,
  //    &texture_desc,
  //    D3D12_RESOURCE_STATE_COPY_DEST,
  //    nullptr,
  //    IID_PPV_ARGS(&trs.texture_buffer)));
  //
  //  const uint64_t buffer_size = GetRequiredIntermediateSize(trs.texture_buffer.Get(), 0, 1);
  //
  //  // Create the GPU upload buffer.
  //  create_upload_resource(device, buffer_size, trs.texture_buffer_upload);
  //
  //  // Copy data to the intermediate upload heap and then schedule a copy 
  //  // from the upload heap to the Texture2D.
  //  D3D12_SUBRESOURCE_DATA texture_data = {};
  //  texture_data.pData = trs.is_hdr ? reinterpret_cast<void*>(trs.data_hdr.data()) : trs.data_ldr.data();
  //  texture_data.RowPitch = texture->width * texture->channels * (trs.is_hdr ? sizeof(float) : sizeof(uint8_t));
  //  texture_data.SlicePitch = texture_data.RowPitch * texture->height;
  //
  //  UpdateSubresources(command_list.Get(), trs.texture_buffer.Get(), trs.texture_buffer_upload.Get(), 0, 0, 1, &texture_data);
  //  
  //  resource_barrier(command_list, trs.texture_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  //  
  //  // Describe and create a SRV for the texture.
  //  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  //  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  //  srv_desc.Format = texture_desc.Format;
  //  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  //  srv_desc.Texture2D.MipLevels = 1;
  //  device->CreateShaderResourceView(texture->render_state.texture_buffer.Get(), &srv_desc, handle);
  //
  //  trs.is_resource_online = true;
  //}

  bool fdx12::get_dxc_hash(ComPtr<IDxcResult> result, std::string& out_hash)
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
  
  bool fdx12::get_dxc_blob(ComPtr<IDxcResult> result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlob>& out_blob)
  {
    ComPtr<IDxcBlobUtf16> name = nullptr;
    if(FAILED(result->GetOutput(blob_type, IID_PPV_ARGS(out_blob.GetAddressOf()), name.GetAddressOf())) && out_blob != nullptr)
    {
      LOG_ERROR("Unable to get dxc blob {0}", static_cast<int32_t>(blob_type));
      return false;
    }
    return true;
  }
  
  bool fdx12::get_dxc_blob(ComPtr<IDxcResult> result, DXC_OUT_KIND blob_type, ComPtr<IDxcBlobUtf8>& out_blob)
  {
    ComPtr<IDxcBlobUtf16> name = nullptr;
    if(FAILED(result->GetOutput(blob_type, IID_PPV_ARGS(out_blob.GetAddressOf()), name.GetAddressOf())) && out_blob != nullptr)
    {
      LOG_ERROR("Unable to get dxc blob {0}", static_cast<int32_t>(blob_type));
      return false;
    }
    return true;
  } 
  
  bool fdx12::save_dxc_blob(ComPtr<IDxcBlob> blob, const char* path)
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