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
  void fdx12::get_hw_adapter(IDXGIFactory1* factory, IDXGIAdapter1** out_adapter, bool prefer_high_performance_adapter)
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

  void fdx12::create_device(ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12Device2>& out_device)
  {
    const D3D_SHADER_MODEL required_shader_model = D3D_SHADER_MODEL_6_0;                
    const D3D_FEATURE_LEVEL required_feature_level = D3D_FEATURE_LEVEL_12_0;            
    
    //const UUID experimental_features[] = { D3D12ExperimentalShaderModels };
    //THROW_IF_FAILED(D3D12EnableExperimentalFeatures(1, experimental_features, nullptr, nullptr));
    
    ComPtr<IDXGIAdapter1> adapter1;
    get_hw_adapter(factory.Get(), &adapter1, true);
    DXGI_ADAPTER_DESC adapter_desc;
    adapter1->GetDesc(&adapter_desc);
    LOG_INFO("Graphics Device: {0}", fstring_tools::to_utf8(adapter_desc.Description));
    
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
      
      THROW_IF_FAILED((D3D12CreateDevice(adapter1.Get(), feature_level.MaxSupportedFeatureLevel, IID_PPV_ARGS(out_device.GetAddressOf()))));
    }
  }

  void fdx12::create_command_queue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& out_command_queue)
  {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    THROW_IF_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(out_command_queue.GetAddressOf())))
  }

  void fdx12::create_swap_chain(HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue, uint32_t back_buffer_count, bool allow_screen_tearing, ComPtr<IDXGISwapChain4>& out_swap_chain)
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
    THROW_IF_FAILED(factory->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &desc, nullptr, nullptr, &swap_chain1))
    THROW_IF_FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    THROW_IF_FAILED(swap_chain1.As(&out_swap_chain))
  }

  void fdx12::create_render_target_descriptor_heap(ComPtr<ID3D12Device> device, uint32_t back_buffer_count, ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap)
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = back_buffer_count;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;
    THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_descriptor_heap.GetAddressOf())))
  }
  
  void fdx12::create_render_target(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swap_chain, ComPtr<ID3D12DescriptorHeap> descriptor_heap, uint32_t back_buffer_count, std::vector<ComPtr<ID3D12Resource>>& out_rtv)
  {
    const uint32_t descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(descriptor_heap->GetCPUDescriptorHandleForHeapStart());
    out_rtv.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      ComPtr<ID3D12Resource> back_buffer;
      THROW_IF_FAILED(swap_chain->GetBuffer(n, IID_PPV_ARGS(back_buffer.GetAddressOf())))
      device->CreateRenderTargetView(back_buffer.Get(), nullptr, handle);
      out_rtv.push_back(back_buffer);
      handle.Offset(static_cast<int>(descriptor_size));
    }
  }

  void fdx12::create_depth_stencil_descriptor_heap(ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap)
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&out_descriptor_heap)));
  }

  void fdx12::resize_swap_chain(ComPtr<IDXGISwapChain4> swap_chain, uint32_t back_buffer_count, uint32_t width, uint32_t height)
  {
    DXGI_SWAP_CHAIN_DESC desc = {};
    THROW_IF_FAILED(swap_chain->GetDesc(&desc));
    THROW_IF_FAILED(swap_chain->ResizeBuffers(back_buffer_count, width, height, desc.BufferDesc.Format, desc.Flags));
  }
  
  void fdx12::create_depth_stencil(ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap> descriptor_heap, uint32_t width, uint32_t height, uint32_t back_buffer_count, std::vector<ComPtr<ID3D12Resource>>& out_dsv)
  {
    D3D12_CLEAR_VALUE clear_value = {};
    clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    clear_value.DepthStencil = { 1.0f, 0 };

    CD3DX12_HEAP_PROPERTIES heap_prop(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    const uint32_t descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    out_dsv.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      ComPtr<ID3D12Resource> back_buffer;
      THROW_IF_FAILED(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(back_buffer.GetAddressOf())));
      D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
      desc.Format = DXGI_FORMAT_D32_FLOAT;
      desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MipSlice = 0;
      desc.Flags = D3D12_DSV_FLAG_NONE;
      device->CreateDepthStencilView(back_buffer.Get(), &desc, handle);
      out_dsv.push_back(back_buffer);
      handle.Offset(descriptor_size);
    }
  }

  void fdx12::create_cbv_srv_uav_descriptor_heap(ComPtr<ID3D12Device> device, fdescriptor_heap& out_descriptor_heap)
  {
    out_descriptor_heap.descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = MAX_MAIN_DESCRIPTORS;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_descriptor_heap.heap.GetAddressOf())))
  }

  void fdx12::create_command_list(ComPtr<ID3D12Device> device, uint32_t back_buffer_count, ComPtr<ID3D12GraphicsCommandList>& out_command_list, std::vector<ComPtr<ID3D12CommandAllocator>>& out_command_allocators)
  {
    out_command_allocators.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      out_command_allocators.push_back(nullptr);
      THROW_IF_FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(out_command_allocators[n].GetAddressOf())))
    }
    THROW_IF_FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, out_command_allocators[0].Get(), nullptr, IID_PPV_ARGS(out_command_list.GetAddressOf())))
  }

  void fdx12::create_synchronisation(ComPtr<ID3D12Device> device, uint32_t back_buffer_count, uint64_t initial_fence_value, ComPtr<ID3D12Fence>& out_fence, HANDLE& out_fence_event, std::vector<uint64_t>& out_fence_values)
  {
    THROW_IF_FAILED(device->CreateFence(initial_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(out_fence.GetAddressOf())))

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

  void fdx12::set_render_targets(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap, ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap, int back_buffer_index)
  {
    const uint32_t rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t dsv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), back_buffer_index, rtv_descriptor_size);
    const CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle(dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), back_buffer_index, dsv_descriptor_size);
    command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);
  }
  
  void fdx12::set_viewport(ComPtr<ID3D12GraphicsCommandList> command_list, uint32_t width, uint32_t height)
  {
    CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    command_list->RSSetViewports(1, &viewport);
  }

  void fdx12::set_scissor(ComPtr<ID3D12GraphicsCommandList> command_list, uint32_t width, uint32_t height)
  {
    CD3DX12_RECT scissor_rect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
    command_list->RSSetScissorRects(1, &scissor_rect);
  }

  void fdx12::clear_render_target(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap, uint32_t back_buffer_index)
  {
    const uint32_t descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), back_buffer_index, descriptor_size);
    command_list->ClearRenderTargetView(handle, DirectX::Colors::LightSlateGray, 0, nullptr);
  }

  void fdx12::clear_depth_stencil(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap, uint32_t back_buffer_index)
  {
    const uint32_t descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    const CD3DX12_CPU_DESCRIPTOR_HANDLE handle(dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), back_buffer_index, descriptor_size);
    command_list->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
  }
  
  void fdx12::report_live_objects()
  {
    ComPtr<IDXGIDebug1> debug;
    THROW_IF_FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(debug.GetAddressOf())))
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
  }

  void fdx12::resource_barrier(ComPtr<ID3D12GraphicsCommandList> command_list, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after)
  {
    const CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), state_before, state_after);
    command_list->ResourceBarrier(1, &resource_barrier);
  }

  void fdx12::upload_buffer_resource(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, uint64_t buffer_size, const void* in_buffer, ComPtr<ID3D12Resource>& out_upload_intermediate, ComPtr<ID3D12Resource>& out_gpu_resource)
  {
    if (in_buffer)
    {
      create_default_resource(device, buffer_size, out_gpu_resource);
      create_upload_resource(device, buffer_size, out_upload_intermediate);
      
      D3D12_SUBRESOURCE_DATA data;
      data.pData = in_buffer;
      data.RowPitch = static_cast<uint32_t>(buffer_size);
      data.SlicePitch = data.RowPitch;

      UpdateSubresources(command_list.Get(), out_gpu_resource.Get(), out_upload_intermediate.Get(), 0, 0, 1, &data);
    }
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

  void fdx12::create_default_resource(ComPtr<ID3D12Device> device, uint64_t buffer_size, ComPtr<ID3D12Resource>& out_resource)
  {
    const CD3DX12_HEAP_PROPERTIES type_default = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size, D3D12_RESOURCE_FLAG_NONE);
    THROW_IF_FAILED(device->CreateCommittedResource(
      &type_default,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(out_resource.GetAddressOf())));
  }

  void fdx12::create_const_buffer(ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle, uint64_t buffer_size, ComPtr<ID3D12Resource>& out_resource)
  {
    // https://logins.github.io/graphics/2020/07/31/DX12ResourceHandling.html#resource-mapping
    // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables#c0
    
    uint64_t size_aligned = fdx12::align_size_to(buffer_size, 255);
    
    fdx12::create_upload_resource(device, size_aligned, out_resource);
    
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
    cbv_desc.BufferLocation = out_resource->GetGPUVirtualAddress();
    cbv_desc.SizeInBytes = static_cast<uint32_t>(size_aligned);
    device->CreateConstantBufferView(&cbv_desc, handle);
  }

  void fdx12::update_buffer(ComPtr<ID3D12Resource> resource, uint64_t buffer_size, const void* in_buffer)
  {
    CD3DX12_RANGE read_range(0, 0);
    uint8_t* mapping = nullptr;
    THROW_IF_FAILED(resource->Map(0, &read_range, reinterpret_cast<void**>(&mapping)));
    memcpy(mapping, in_buffer, buffer_size);
    resource->Unmap(0, nullptr);
  }

  void fdx12::create_shader_resource_buffer(ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle, uint64_t buffer_size, ComPtr<ID3D12Resource>& out_resource)
  {
    uint64_t aligned_size = fdx12::align_size_to(buffer_size, 255);
    
    fdx12::create_upload_resource(device, aligned_size, out_resource);

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Buffer.NumElements = 1;
    srv_desc.Buffer.StructureByteStride = static_cast<uint32_t>(aligned_size);
    srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    device->CreateShaderResourceView(out_resource.Get(), &srv_desc, handle);
  }

  void fdx12::upload_vertex_buffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, fstatic_mesh_render_state& out_render_state)
  {
    const uint64_t vertex_list_size = out_render_state.vertex_list.size() * sizeof(fvertex_data);
    
    upload_buffer_resource(device, command_list, vertex_list_size, out_render_state.vertex_list.data(), out_render_state.vertex_buffer_upload, out_render_state.vertex_buffer);
    
    out_render_state.vertex_buffer_view.BufferLocation = out_render_state.vertex_buffer->GetGPUVirtualAddress();
    out_render_state.vertex_buffer_view.SizeInBytes = static_cast<uint32_t>(vertex_list_size);
    out_render_state.vertex_buffer_view.StrideInBytes = sizeof(fvertex_data);

    resource_barrier(command_list, out_render_state.vertex_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    
    out_render_state.is_resource_online = true;
  }

  void fdx12::upload_index_buffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, fstatic_mesh_render_state& out_render_state)
  {
    const uint64_t face_list_size = out_render_state.face_list.size() * sizeof(fface_data); 

    upload_buffer_resource(device, command_list, face_list_size, out_render_state.face_list.data(), out_render_state.index_buffer_upload, out_render_state.index_buffer);
    
    out_render_state.index_buffer_view.BufferLocation = out_render_state.index_buffer->GetGPUVirtualAddress();
    out_render_state.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    out_render_state.index_buffer_view.SizeInBytes = static_cast<uint32_t>(face_list_size);

    resource_barrier(command_list, out_render_state.index_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    
    out_render_state.is_resource_online = true;
  }

  void fdx12::upload_texture_buffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, const CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, atexture* texture)
  {
    ftexture_render_state& trs = texture->render_state;
    
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC texture_desc = {};
    texture_desc.MipLevels = 1;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // TODO move to texture, make persistent
    texture_desc.Width = texture->width;
    texture_desc.Height = texture->height;
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
      IID_PPV_ARGS(&trs.texture_buffer)));

    const uint64_t buffer_size = GetRequiredIntermediateSize(trs.texture_buffer.Get(), 0, 1);

    // Create the GPU upload buffer.
    create_upload_resource(device, buffer_size, trs.texture_buffer_upload);

    // Copy data to the intermediate upload heap and then schedule a copy 
    // from the upload heap to the Texture2D.
    D3D12_SUBRESOURCE_DATA texture_data = {};
    texture_data.pData = trs.is_hdr ? reinterpret_cast<void*>(trs.data_hdr.data()) : trs.data_ldr.data();
    texture_data.RowPitch = texture->width * texture->channels * (trs.is_hdr ? sizeof(float) : sizeof(uint8_t));
    texture_data.SlicePitch = texture_data.RowPitch * texture->height;

    UpdateSubresources(command_list.Get(), trs.texture_buffer.Get(), trs.texture_buffer_upload.Get(), 0, 0, 1, &texture_data);
    
    resource_barrier(command_list, trs.texture_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    
    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Format = texture_desc.Format;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(texture->render_state.texture_buffer.Get(), &srv_desc, handle);

    trs.is_resource_online = true;
  }

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

  //void fdx12::create_input_layout(const D3D11_INPUT_ELEMENT_DESC* input_element_desc, uint32_t input_element_desc_size, const ComPtr<ID3D10Blob>& vertex_shader_blob, ComPtr<ID3D11InputLayout>& input_layout) const
  //{
  //  THROW_IF_FAILED(device->CreateInputLayout(input_element_desc, input_element_desc_size, vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), input_layout.GetAddressOf()))
  //}
  //
  //void fdx12::create_sampler_state(ComPtr<ID3D11SamplerState>& out_sampler_state) const
  //{
  //  D3D11_SAMPLER_DESC desc = {};
  //  desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  //  desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  //  desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  //  desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  //  desc.BorderColor[0] = 1.0f;
  //  desc.BorderColor[1] = 1.0f;
  //  desc.BorderColor[2] = 1.0f;
  //  desc.BorderColor[3] = 1.0f;
  //  desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  //  THROW_IF_FAILED(device->CreateSamplerState(&desc, &out_sampler_state))
  //}
  //
  //void fdx12::create_constant_buffer(uint32_t size, ComPtr<ID3D11Buffer>& out_constant_buffer) const
  //{
  //  D3D11_BUFFER_DESC desc = {};
  //  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  //  desc.Usage = D3D11_USAGE_DYNAMIC;
  //  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  //  desc.ByteWidth = size;
  //  THROW_IF_FAILED(device->CreateBuffer(&desc, nullptr, &out_constant_buffer))
  //}
  //
  //void fdx12::create_rasterizer_state(ComPtr<ID3D11RasterizerState>& out_rasterizer_state) const
  //{
  //  D3D11_RASTERIZER_DESC desc = {};
  //  desc.AntialiasedLineEnable = FALSE;
  //  desc.CullMode = D3D11_CULL_BACK;
  //  desc.DepthBias = 0;
  //  desc.DepthBiasClamp = 0.0f;
  //  desc.DepthClipEnable = TRUE;
  //  desc.FillMode = D3D11_FILL_SOLID;
  //  desc.FrontCounterClockwise = FALSE;
  //  desc.MultisampleEnable = FALSE;
  //  desc.ScissorEnable = FALSE;
  //  desc.SlopeScaledDepthBias = 0.0f;
  //  THROW_IF_FAILED(device->CreateRasterizerState(&desc, &out_rasterizer_state))
  //}
  //
  //void fdx12::create_depth_stencil_state(ComPtr<ID3D11DepthStencilState>& out_depth_stencil_state) const
  //{
  //  D3D11_DEPTH_STENCIL_DESC desc = {};
  //  desc.DepthEnable = TRUE;
  //  desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  //  desc.DepthFunc = D3D11_COMPARISON_LESS;
  //  desc.StencilEnable = FALSE;
  //  THROW_IF_FAILED(device->CreateDepthStencilState(&desc, &out_depth_stencil_state))
  //}
  //
  //void fdx12::create_vertex_shader(const ComPtr<ID3D10Blob>& blob, ComPtr<ID3D11VertexShader>& out_vertex_shader) const
  //{
  //  THROW_IF_FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, out_vertex_shader.GetAddressOf()))
  //}
  //
  //void fdx12::create_pixel_shader(const ComPtr<ID3D10Blob>& blob, ComPtr<ID3D11PixelShader>& out_pixel_shader) const
  //{
  //  THROW_IF_FAILED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, out_pixel_shader.GetAddressOf()))
  //}
  //
  //void fdx12::create_index_buffer(const std::vector<fface_data>& in_face_list, ComPtr<ID3D11Buffer>& out_index_buffer) const
  //{
  //  D3D11_BUFFER_DESC desc = {};
  //  desc.ByteWidth = static_cast<int32_t>(in_face_list.size()) * sizeof(fface_data);
  //  desc.Usage = D3D11_USAGE_IMMUTABLE;
  //  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  //  D3D11_SUBRESOURCE_DATA data = {in_face_list.data()};
  //  THROW_IF_FAILED(device->CreateBuffer(&desc, &data, out_index_buffer.GetAddressOf()))
  //}
  //
  //void fdx12::create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, ComPtr<ID3D11Buffer>& out_vertex_buffer) const
  //{
  //  D3D11_BUFFER_DESC desc = {};
  //  desc.ByteWidth = static_cast<int32_t>(in_vertex_list.size()) * sizeof(fvertex_data);
  //  desc.Usage = D3D11_USAGE_IMMUTABLE;
  //  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  //  const D3D11_SUBRESOURCE_DATA data = {in_vertex_list.data()};
  //  THROW_IF_FAILED(device->CreateBuffer(&desc, &data, out_vertex_buffer.GetAddressOf()))
  //}
  //
  //void fdx12::create_render_target_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_RTV_DIMENSION view_dimmension, ComPtr<ID3D11RenderTargetView>& out_render_target_view) const
  //{
  //  D3D11_RENDER_TARGET_VIEW_DESC desc = {};
  //  desc.Format = format;
  //  desc.ViewDimension = view_dimmension;
  //  desc.Texture2D.MipSlice = 0;
  //  THROW_IF_FAILED(device->CreateRenderTargetView(in_texture.Get(), &desc, out_render_target_view.GetAddressOf()))
  //}
  //
  //void fdx12::create_depth_stencil_view(const ComPtr<ID3D11Texture2D>& in_texture, uint32_t width, uint32_t height, ComPtr<ID3D11DepthStencilView>& out_depth_stencil_view) const
  //{
  //  THROW_IF_FAILED(device->CreateDepthStencilView(in_texture.Get(), nullptr, out_depth_stencil_view.GetAddressOf()))) // TODO Missing descriptor
  //}
  //
  //void fdx12::create_texture(uint32_t width, uint32_t height, DXGI_FORMAT format, D3D11_BIND_FLAG bind_flags, D3D11_USAGE usage, ComPtr<ID3D11Texture2D>& out_texture, uint32_t bytes_per_row, const void* in_bytes) const
  //{
  //  D3D11_TEXTURE2D_DESC texture_desc = {};
  //  texture_desc.Width = width;
  //  texture_desc.Height = height;
  //  texture_desc.MipLevels = 1;
  //  texture_desc.ArraySize = 1;
  //  texture_desc.Format = format;
  //  texture_desc.SampleDesc.Count = 1;
  //  texture_desc.Usage = usage;
  //  texture_desc.BindFlags = bind_flags;
  //  texture_desc.CPUAccessFlags = 0;
  //  texture_desc.MiscFlags = 0;
  //
  //  D3D11_SUBRESOURCE_DATA texture_subresource_data = {};
  //  texture_subresource_data.SysMemPitch = bytes_per_row;
  //  texture_subresource_data.pSysMem = in_bytes;
  //  
  //  THROW_IF_FAILED(device->CreateTexture2D(&texture_desc, in_bytes != nullptr ? &texture_subresource_data : nullptr, out_texture.GetAddressOf()))
  //}
  //
  //void fdx12::create_shader_resource_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_SRV_DIMENSION view_dimmension, ComPtr<ID3D11ShaderResourceView>& out_shader_resource_view) const
  //{
  //  D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
  //  shader_resource_view_desc.Format = format;
  //  shader_resource_view_desc.ViewDimension = view_dimmension;
  //  shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
  //  shader_resource_view_desc.Texture2D.MipLevels = 1;
  //  THROW_IF_FAILED(device->CreateShaderResourceView(in_texture.Get(), &shader_resource_view_desc, out_shader_resource_view.GetAddressOf()))
  //}
}