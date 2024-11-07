
#include "d3d12.h"

#include "device.h"

#include <dxgi1_6.h>

#include "d3dx12/d3dx12_core.h"
#include "d3dx12/d3dx12_root_signature.h"

#include "core/exceptions.h"
#include "engine/log.h"
#include "engine/string_tools.h"
#include "renderer/aligned_structs.h"
#include "renderer/descriptor_heap.h"

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

  fdevice fdevice::create(IDXGIFactory4* factory)
  {
    fdevice device;
    constexpr D3D_SHADER_MODEL required_shader_model = D3D_SHADER_MODEL_6_0;                
    constexpr D3D_FEATURE_LEVEL required_feature_level = D3D_FEATURE_LEVEL_12_0;            
    
    //const UUID experimental_features[] = { D3D12ExperimentalShaderModels };
    //THROW_IF_FAILED(D3D12EnableExperimentalFeatures(1, experimental_features, nullptr, nullptr));
    
    ComPtr<IDXGIAdapter1> adapter1;
    get_hw_adapter(factory, &adapter1, true);
    DXGI_ADAPTER_DESC adapter_desc;
    adapter1->GetDesc(&adapter_desc);
    LOG_INFO("Graphics Device: {0}", fstring_tools::to_utf8(adapter_desc.Description))
    
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
      
      THROW_IF_FAILED((D3D12CreateDevice(adapter1.Get(), feature_level.MaxSupportedFeatureLevel, IID_PPV_ARGS(device.device.GetAddressOf()))))
      return device;
    }
  }
  
  void fdevice::create_render_target(IDXGISwapChain4* swap_chain, ID3D12DescriptorHeap* descriptor_heap, uint32_t back_buffer_count, std::vector<ComPtr<ID3D12Resource>>& out_rtv)
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

  void fdevice::create_depth_stencil(ID3D12DescriptorHeap* descriptor_heap, uint32_t width, uint32_t height, ComPtr<ID3D12Resource>& out_dsv)
  {
    D3D12_CLEAR_VALUE clear_value = {};
    clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    clear_value.DepthStencil = { 1.0f, 0 };

    CD3DX12_HEAP_PROPERTIES heap_prop(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    THROW_IF_FAILED(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(out_dsv.GetAddressOf())));
    
    D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;
    desc.Flags = D3D12_DSV_FLAG_NONE;
    device->CreateDepthStencilView(out_dsv.Get(), &desc, descriptor_heap->GetCPUDescriptorHandleForHeapStart());
  }

  void fdevice::create_command_queue(ComPtr<ID3D12CommandQueue>& out_command_queue)
  {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    THROW_IF_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(out_command_queue.GetAddressOf())))
  }

  void fdevice::create_command_list(uint32_t back_buffer_count, ComPtr<ID3D12GraphicsCommandList>& out_command_list, std::vector<ComPtr<ID3D12CommandAllocator>>& out_command_allocators)
  {
    out_command_allocators.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      out_command_allocators.push_back(nullptr);
      THROW_IF_FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(out_command_allocators[n].GetAddressOf())))
    }
    THROW_IF_FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, out_command_allocators[0].Get(), nullptr, IID_PPV_ARGS(out_command_list.GetAddressOf())))
  }

  void fdevice::create_synchronisation(uint32_t back_buffer_count, uint64_t initial_fence_value, ComPtr<ID3D12Fence>& out_fence, HANDLE& out_fence_event, std::vector<uint64_t>& out_fence_values)
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

  void fdevice::create_render_target_descriptor_heap(uint32_t back_buffer_count, ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap)
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = back_buffer_count;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;
    THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_descriptor_heap.GetAddressOf())))
  }
  
  void fdevice::create_depth_stencil_descriptor_heap(ComPtr<ID3D12DescriptorHeap>& out_descriptor_heap)
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&out_descriptor_heap)));
  }

  void fdevice::create_cbv_srv_uav_descriptor_heap(fdescriptor_heap& out_descriptor_heap)
  {
    out_descriptor_heap = fdescriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = MAX_MAIN_DESCRIPTORS;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_descriptor_heap.heap.GetAddressOf())))
  }
  
}