
#include "d3d12.h"
#include <dxgi1_6.h>

#include "d3dx12/d3dx12_core.h"
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_resource_helpers.h"

#include "engine/renderer/device.h"

#include <DirectXColors.h>
#include <sstream>

#include "assets/texture.h"
#include "core/exceptions/windows_error.h"
#include "engine/log.h"
#include "engine/string_tools.h"
#include "engine/math/math.h"
#include "engine/renderer/aligned_structs.h"
#include "engine/renderer/descriptor_heap.h"
#include "engine/renderer/dx12_lib.h"
#include "engine/renderer/gpu_resources.h"
#include "engine/renderer/pipeline_state.h"

namespace engine
{
  void fdevice::get_hw_adapter(IDXGIFactory1* factory, IDXGIAdapter1** out_adapter, bool prefer_high_performance_adapter)
  {
    *out_adapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter1;
    ComPtr<IDXGIFactory6> factory6;

    if(SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(factory6.GetAddressOf()))))
    {
      for(int adapter_index = 0;
          SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapter_index,prefer_high_performance_adapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(adapter1.GetAddressOf())));
          ++adapter_index)
      {
        DXGI_ADAPTER_DESC1 desc;
        adapter1->GetDesc1(&desc);

        if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
          continue;
        }

        if(SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
          break;
        }
      }
    }

    if(adapter1.Get() == nullptr)
    {
      for(int adapter_index = 0; SUCCEEDED(factory->EnumAdapters1(adapter_index, &adapter1)); ++adapter_index)
      {
        DXGI_ADAPTER_DESC1 desc;
        adapter1->GetDesc1(&desc);

        if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
          continue;
        }

        if(SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
          break;
        }
      }
    }

    if(adapter1.Get() == nullptr)
    {
      throw std::runtime_error("EnumAdapterByGpuPreference and EnumAdapters1 failed.");
    }

    *out_adapter = adapter1.Detach();
  }

  fdevice* fdevice::create(IDXGIFactory4* factory)
  {
    fdevice* device = new fdevice();
    constexpr D3D_SHADER_MODEL required_shader_model = D3D_SHADER_MODEL_6_0;                
    constexpr D3D_FEATURE_LEVEL required_feature_level = D3D_FEATURE_LEVEL_12_0;            
    
    //const UUID experimental_features[] = { D3D12ExperimentalShaderModels };
    //THROW_IF_FAILED(D3D12EnableExperimentalFeatures(1, experimental_features, nullptr, nullptr));
    
    ComPtr<IDXGIAdapter1> adapter1;
    get_hw_adapter(factory, &adapter1, true);
    DXGI_ADAPTER_DESC adapter_desc;
    adapter1->GetDesc(&adapter_desc);
    std::string adapter_name =  fstring_tools::to_utf8(adapter_desc.Description);
    LOG_INFO("Graphics Device: {0}", adapter_name)
#if USE_NSIGHT_GRAPHICS
    if(!fstring_tools::contains(adapter_name, "NVIDIA"))
    {
      throw std::runtime_error("No NVIDIA GPU detected, Nsight Graphics will not work properly!");
    }
#endif
    
    ComPtr<ID3D12Device2> temp_device;  // Used only to check feature level and shader model
    THROW_IF_FAILED((D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(temp_device.GetAddressOf()))))

    // Check shader model
    {
      D3D12_FEATURE_DATA_SHADER_MODEL shader_model = { required_shader_model };
      if (FAILED(temp_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model, sizeof(shader_model))) || (shader_model.HighestShaderModel < required_shader_model))
      {
        throw std::runtime_error("Shader model is not high enough!");
      }
    }

    // Check feature support and create highest possible device
    {
      const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_2 };
      
      D3D12_FEATURE_DATA_FEATURE_LEVELS feature_level = {};
      feature_level.pFeatureLevelsRequested = feature_levels;
      feature_level.NumFeatureLevels = _countof(feature_levels);
      
      if(FAILED(temp_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_level, sizeof(feature_level))) || feature_level.MaxSupportedFeatureLevel < required_feature_level)
      {
        throw std::runtime_error("Feature level is not high enough!");
      }
      
      THROW_IF_FAILED((D3D12CreateDevice(adapter1.Get(), feature_level.MaxSupportedFeatureLevel, IID_PPV_ARGS(device->com.GetAddressOf()))))
      DX_RELEASE(adapter1)
      return device;
    }
  }

  void fdevice::create_root_signature(const std::vector<CD3DX12_ROOT_PARAMETER1>& root_parameters, const std::vector<CD3DX12_STATIC_SAMPLER_DESC>& static_samplers, D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags, ComPtr<ID3D12RootSignature>& out_root_signature, const char* name) const
  {
    D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data = {};
    feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(com->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(feature_data))))
    {
      feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
      
    // Serialize the root signature.
    ComPtr<ID3DBlob> root_signature_serialized;
    ComPtr<ID3DBlob> error_blob;
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc;
    root_signature_desc.Init_1_1(fmath::to_uint32(root_parameters.size()), root_parameters.data(), fmath::to_uint32(static_samplers.size()), static_samplers.data(), root_signature_flags);
    
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
    THROW_IF_FAILED(com->CreateRootSignature(0, root_signature_serialized->GetBufferPointer(), root_signature_serialized->GetBufferSize(), IID_PPV_ARGS(out_root_signature.GetAddressOf())));
#if BUILD_DEBUG
    DX_SET_NAME(out_root_signature, "Root signature: {}", name)
#endif
  }

  void fdevice::create_pipeline_state(fpipeline_state_stream& pipeline_state_stream, ComPtr<ID3D12PipelineState>& out_pipeline_state, const char* name) const
  {
    const D3D12_PIPELINE_STATE_STREAM_DESC pipeline_state_stream_desc(sizeof(fpipeline_state_stream), &pipeline_state_stream);
    THROW_IF_FAILED(com->CreatePipelineState(&pipeline_state_stream_desc, IID_PPV_ARGS(out_pipeline_state.GetAddressOf())));
#if BUILD_DEBUG
    DX_SET_NAME(out_pipeline_state, "Pipeline state: {}", name)
#endif
  }

  void fdevice::create_pipeline_state(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc, ComPtr<ID3D12PipelineState>& out_pipeline_state, const char* name) const
  {
    THROW_IF_FAILED(com->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(out_pipeline_state.GetAddressOf())));
#if BUILD_DEBUG
    DX_SET_NAME(out_pipeline_state, "Pipeline state: {}", name)
#endif
  }

  void fdevice::create_command_queue(ComPtr<ID3D12CommandQueue>& out_command_queue) const 
  {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    THROW_IF_FAILED(com->CreateCommandQueue(&desc, IID_PPV_ARGS(out_command_queue.GetAddressOf())))
  }

  void fdevice::create_command_list(uint32_t back_buffer_count, ComPtr<ID3D12GraphicsCommandList>& out_command_list, std::vector<ComPtr<ID3D12CommandAllocator>>& out_command_allocators) const 
  {
    out_command_allocators.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      out_command_allocators.push_back(nullptr);
      THROW_IF_FAILED(com->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(out_command_allocators[n].GetAddressOf())))
    }
    THROW_IF_FAILED(com->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, out_command_allocators[0].Get(), nullptr, IID_PPV_ARGS(out_command_list.GetAddressOf())))
  }

  void fdevice::create_synchronisation(uint32_t back_buffer_count, uint64_t initial_fence_value, ComPtr<ID3D12Fence>& out_fence, HANDLE& out_fence_event, std::vector<uint64_t>& out_fence_values) const 
  {
    THROW_IF_FAILED(com->CreateFence(initial_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(out_fence.GetAddressOf())))

    out_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if(out_fence_event == nullptr)
    {
      THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()))
    }

    out_fence_values.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      out_fence_values.push_back(initial_fence_value);
    }
  }

  void fdevice::create_render_target_descriptor_heap(fdescriptor_heap& out_descriptor_heap, const char* name) const 
  {
    out_descriptor_heap = fdescriptor_heap(com.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, MAX_RTV_DESCRIPTORS);

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = MAX_RTV_DESCRIPTORS;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;
    THROW_IF_FAILED(com->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_descriptor_heap.com.GetAddressOf())))
    
#if BUILD_DEBUG
    DX_SET_NAME(out_descriptor_heap.com, "RTV heap: {}", name)
#endif
  }
  
  void fdevice::create_depth_stencil_descriptor_heap(fdescriptor_heap& out_descriptor_heap, const char* name) const 
  {
    out_descriptor_heap = fdescriptor_heap(com.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, MAX_DSV_DESCRIPTORS);

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = MAX_DSV_DESCRIPTORS;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    THROW_IF_FAILED(com->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_descriptor_heap.com.GetAddressOf())));

#if BUILD_DEBUG
    DX_SET_NAME(out_descriptor_heap.com, "DSV heap: {}", name)
#endif
  }

  void fdevice::create_cbv_srv_uav_descriptor_heap(fdescriptor_heap& out_descriptor_heap, const char* name) const
  {
    out_descriptor_heap = fdescriptor_heap(com.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MAX_MAIN_DESCRIPTORS);
    
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = MAX_MAIN_DESCRIPTORS;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    THROW_IF_FAILED(com->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_descriptor_heap.com.GetAddressOf())))

#if BUILD_DEBUG
    DX_SET_NAME(out_descriptor_heap.com, "CBV SRV UAV heap: {}", name)
#endif
  }

  void fdevice::create_const_buffer(fdescriptor_heap* heap, uint32_t in_size, fconst_buffer& out_buffer, const char* name) const
  {
    out_buffer.size = fdx12::align_size_to(in_size, 255);
    heap->push(out_buffer.cbv, name);

    // https://logins.github.io/graphics/2020/07/31/DX12ResourceHandling.html#resource-mapping
    // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables#c0
    
    create_upload_resource(out_buffer.size, out_buffer.resource);
    
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
    cbv_desc.BufferLocation = out_buffer.resource->GetGPUVirtualAddress();
    cbv_desc.SizeInBytes = out_buffer.size;
    com->CreateConstantBufferView(&cbv_desc, out_buffer.cbv.cpu_descriptor_handle);
    
#if BUILD_DEBUG
    DX_SET_NAME(out_buffer.resource, "{}", name)
#endif
  }

  void fdevice::create_shader_resource_buffer(fdescriptor_heap* heap, uint32_t in_size, fshader_resource_buffer& out_buffer, const char* name) const
  {
    out_buffer.size = fdx12::align_size_to(in_size, 255);
    heap->push(out_buffer.srv, name);
    
    create_upload_resource(out_buffer.size, out_buffer.resource);

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Buffer.NumElements = 1;
    srv_desc.Buffer.StructureByteStride = out_buffer.size;
    srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    com->CreateShaderResourceView(out_buffer.resource.Get(), &srv_desc, out_buffer.srv.cpu_descriptor_handle);
    
#if BUILD_DEBUG
    DX_SET_NAME(out_buffer.resource, "{}", name)
#endif
  }

  void fdevice::create_back_buffer(IDXGISwapChain4* swap_chain, uint32_t swap_chain_buffer_id, fdescriptor_heap& descriptor_heap, ftexture_resource& out_rtv, const char* name) const
  {
    if(out_rtv.com)
    {
      throw std::runtime_error("Overwriting object in existing COM pointer (create_back_buffer)");
    }
    
    THROW_IF_FAILED(swap_chain->GetBuffer(swap_chain_buffer_id, IID_PPV_ARGS(out_rtv.com.GetAddressOf())))
    descriptor_heap.push(out_rtv.rtv, name);
    com->CreateRenderTargetView(out_rtv.com.Get(), nullptr, out_rtv.rtv.cpu_descriptor_handle);

#if BUILD_DEBUG
    DX_SET_NAME(out_rtv.com, "Back buffer: {}", name)
#endif
}

  void fdevice::create_frame_buffer(fdescriptor_heap* main_heap, fdescriptor_heap* rtv_heap, ftexture_resource* out_texture, uint32_t width, uint32_t height, DXGI_FORMAT format, D3D12_RESOURCE_STATES initial_state, const char* name) const
  {
    const CD3DX12_CLEAR_VALUE clear_color = { format, DirectX::Colors::Black };
    const CD3DX12_HEAP_PROPERTIES default_heap(D3D12_HEAP_TYPE_DEFAULT);

    if(out_texture->com)
    {
      throw std::runtime_error("Overwriting object in existing COM pointer (create_frame_buffer)");
    }
    
    D3D12_RESOURCE_DESC desc = {};
    desc.MipLevels = 1;
    desc.Format = format;
    desc.Width = width;
    desc.Height = height;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    THROW_IF_FAILED(com->CreateCommittedResource(
      &default_heap,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      initial_state,
      &clear_color,
      IID_PPV_ARGS(out_texture->com.GetAddressOf())));

    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = format;
    rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtv_heap->push(out_texture->rtv, name);
    com->CreateRenderTargetView(out_texture->com.Get(), &rtv_desc, out_texture->rtv.cpu_descriptor_handle);
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = format;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;
    main_heap->push(out_texture->srv, name);
    com->CreateShaderResourceView(out_texture->com.Get(), &srv_desc, out_texture->srv.cpu_descriptor_handle);

#if BUILD_DEBUG
    DX_SET_NAME(out_texture->com, "Frame buffer: {}", name)
#endif
  }

  void fdevice::create_depth_stencil(fdescriptor_heap* dsv_heap, ftexture_resource* out_texture, uint32_t width, uint32_t height, DXGI_FORMAT format,  D3D12_RESOURCE_STATES initial_state, const char* name) const
  {
    if(out_texture->com)
    {
      throw std::runtime_error("Overwriting object in existing COM pointer (create_depth_stencil)");
    }
    
    CD3DX12_CLEAR_VALUE clear_value = {format, 1.0f, 0};
    const CD3DX12_HEAP_PROPERTIES default_heap(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC desc = {};
    desc.MipLevels = 1;
    desc.Format = format;
    desc.Width = width;
    desc.Height = height;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    THROW_IF_FAILED(com->CreateCommittedResource(
      &default_heap,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      initial_state,
      &clear_value,
      IID_PPV_ARGS(out_texture->com.GetAddressOf())));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
    dsv_desc.Format = format;
    dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsv_heap->push(out_texture->dsv, name);
    com->CreateDepthStencilView(out_texture->com.Get(), &dsv_desc, out_texture->dsv.cpu_descriptor_handle);

#if BUILD_DEBUG
    DX_SET_NAME(out_texture->com, "Depth stencil: {}", name)
#endif
  }
  
  void fdevice::create_texture_buffer(fdescriptor_heap* heap, ftexture_resource& out_resource, uint32_t width, uint32_t height, DXGI_FORMAT format, const char* name) const
  {
    if(out_resource.com)
    {
      throw std::runtime_error("Overwriting object in existing COM pointer (create_texture_buffer)");
    }
    
    heap->push(out_resource.srv, name);

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
    THROW_IF_FAILED(com->CreateCommittedResource(
      &default_heap,
      D3D12_HEAP_FLAG_NONE,
      &texture_desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(out_resource.com.GetAddressOf())));

    const uint32_t buffer_size = fmath::to_uint32(GetRequiredIntermediateSize(out_resource.com.Get(), 0, 1));
    create_upload_resource(buffer_size, out_resource.upload_com);
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Format = texture_desc.Format;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    com->CreateShaderResourceView(out_resource.com.Get(), &srv_desc, out_resource.srv.cpu_descriptor_handle);
    
#if BUILD_DEBUG
    DX_SET_NAME(out_resource.com, "Texture: {}", name)
    DX_SET_NAME(out_resource.upload_com, "Texture upload: {}", name)
#endif
  }
  
  void fdevice::create_texture_buffer(fdescriptor_heap* heap, atexture* texture_asset, const char* name) const
  {
    create_texture_buffer(heap, texture_asset->gpu_resource, texture_asset->width, texture_asset->height, texture_asset->format, name);
  }

  void fdevice::create_upload_resource(uint32_t buffer_size, ComPtr<ID3D12Resource>& out_resource) const
  {
    if(out_resource)
    {
      throw std::runtime_error("Overwriting object in existing COM pointer (create_upload_resource)");
    }
    
    const CD3DX12_HEAP_PROPERTIES type_upload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size);
    THROW_IF_FAILED(com->CreateCommittedResource(
      &type_upload,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(out_resource.GetAddressOf())));
  }

  void fdevice::create_buffer_resource(uint32_t buffer_size, ComPtr<ID3D12Resource>& out_resource) const
  {
    if(out_resource)
    {
      throw std::runtime_error("Overwriting object in existing COM pointer (create_buffer_resource)");
    }
    
    const CD3DX12_HEAP_PROPERTIES type_default = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size, D3D12_RESOURCE_FLAG_NONE);
    THROW_IF_FAILED(com->CreateCommittedResource(
      &type_default,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(out_resource.GetAddressOf())));
  }

  DXGI_SAMPLE_DESC fdevice::get_multisample_quality_levels(DXGI_FORMAT format, uint32_t num_samples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const
  {
    DXGI_SAMPLE_DESC desc = { 1, 0 };

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS quality_levels;
    quality_levels.Format           = format;
    quality_levels.SampleCount      = 1;
    quality_levels.Flags            = flags;
    quality_levels.NumQualityLevels = 0;

    while (quality_levels.SampleCount <= num_samples
        && SUCCEEDED(com->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &quality_levels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)))
        && quality_levels.NumQualityLevels > 0)
    {
      desc.Count   = quality_levels.SampleCount;
      desc.Quality = quality_levels.NumQualityLevels - 1;
      quality_levels.SampleCount *= 2;
    }
    return desc;
  }
  
  void fdevice::enable_info_queue() const
  {
    ComPtr<ID3D12InfoQueue> info_queue;
    com->QueryInterface(IID_PPV_ARGS(info_queue.GetAddressOf()));
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    LOG_DEBUG("Enabled info queue")
  }
}