

#include <winerror.h>
#include <cassert>
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_barriers.h"

#include "renderer/dx12_lib.h"

#include "core/exceptions.h"
#include "engine/log.h"
#include "engine/string_tools.h"

namespace engine
{
  void fdx12::get_hw_adapter(IDXGIFactory1* in_factory, IDXGIAdapter1** out_adapter, bool prefer_high_performance_adapter)
  {
    *out_adapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter1;
    ComPtr<IDXGIFactory6> factory6;
    
    if (SUCCEEDED(in_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
      for(UINT adapter_index = 0;
          SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapter_index,prefer_high_performance_adapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter1)));
          ++adapter_index)
      {
        DXGI_ADAPTER_DESC1 desc;
        adapter1->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
          continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
          break;
        }
      }
    }

    if(adapter1.Get() == nullptr)
    {
      for (UINT adapter_index = 0; SUCCEEDED(in_factory->EnumAdapters1(adapter_index, &adapter1)); ++adapter_index)
      {
        DXGI_ADAPTER_DESC1 desc;
        adapter1->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
          continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
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
  
  void fdx12::create_pipeline(HWND hwnd)
  {
    // Debug layer
#if BUILD_DEBUG
    ComPtr<ID3D12Debug> debug;
    ComPtr<ID3D12Debug1> debug1;
    THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))
    THROW_IF_FAILED(debug->QueryInterface(IID_PPV_ARGS(&debug1)))
    debug1->SetEnableGPUBasedValidation(true);
    debug->EnableDebugLayer();
    DX_RELEASE(debug);
    DX_RELEASE(debug1);
#endif
    
    // Factory, hardware adapter and device
    ComPtr<IDXGIFactory4> factory;
    {
      ComPtr<IDXGIAdapter1> adapter;

      UINT factory_flags = 0;
#if BUILD_DEBUG
      factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
      THROW_IF_FAILED(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)))
      
      get_hw_adapter(factory.Get(), &adapter);
      DXGI_ADAPTER_DESC adapter_desc;
      adapter->GetDesc(&adapter_desc);
      LOG_INFO("Graphics Device: {0}", fstring_tools::to_utf8(adapter_desc.Description));
      THROW_IF_FAILED((D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
      DX_RELEASE(adapter);
    }

    // Info queue
#if BUILD_DEBUG
    {
      ID3D12InfoQueue* info_queue = nullptr;
      device->QueryInterface(IID_PPV_ARGS(&info_queue));
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
      info_queue->Release();
    }
#endif

    // Command queue
    {
      D3D12_COMMAND_QUEUE_DESC desc = {};
      desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      THROW_IF_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_queue)) < 0)
    }

    // Swap chain
    {
      DXGI_SWAP_CHAIN_DESC1 desc = {};
      desc.BufferCount = back_buffer_count;
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
      THROW_IF_FAILED(swap_chain1.As(&swap_chain))
      DX_RELEASE(swap_chain1);
      DX_RELEASE(factory);
    }

    // Rtv descriptor heap, descriptor handle, buffer and rtv
    {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.NumDescriptors = back_buffer_count;
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtv_descriptor_heap)))
      
      rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

      D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
      for (UINT n = 0; n < back_buffer_count; n++)
      {
        THROW_IF_FAILED(swap_chain->GetBuffer(n, IID_PPV_ARGS(&rtv[n])))
        device->CreateRenderTargetView(rtv[n].Get(), nullptr, handle);
        handle.ptr += rtv_descriptor_size;
      }
    }

    // Render target shader resource descriptor heap
    {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.NumDescriptors = 1;
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
      THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srv_descriptor_heap)))
    }

    // Command allocators, and lists
    {
      for (UINT n = 0; n < back_buffer_count; n++)
      {
        THROW_IF_FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator[n])))
      }
      THROW_IF_FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator[0].Get(), nullptr, IID_PPV_ARGS(&command_list)))
      THROW_IF_FAILED(command_list->Close())
    }
    
    // Create an empty root signature.
    {
      CD3DX12_ROOT_SIGNATURE_DESC desc;
      desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      ComPtr<ID3DBlob> signature;
      ComPtr<ID3DBlob> error;
      THROW_IF_FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))
      THROW_IF_FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)))
    }

    // TODO Create pipeline state (shaders, input, etc)
    
    // TODO Create vertex buffer

    // Create synchronization objects
    {
      THROW_IF_FAILED(device->CreateFence(fence_value[back_buffer_index], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))

      fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
      if (fence_event == nullptr)
      {
        THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()))
      }
    }

    wait_for_gpu();
  }

  void fdx12::move_to_next_frame()
  {
    UINT64 current_fence_value = fence_value[back_buffer_index];
    command_queue->Signal(fence.Get(), current_fence_value);
    
    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();

    int completed_fence_value = fence->GetCompletedValue();

    // Wait if all backbuffers are in use (currently displayed or rendered on GPU)
    if (completed_fence_value < fence_value[back_buffer_index])
    {
      THROW_IF_FAILED(fence->SetEventOnCompletion(fence_value[back_buffer_index], fence_event))
      WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
    }

    // Set things up for next frame
    fence_value[back_buffer_index] = current_fence_value + 1;
  }

  void fdx12::wait_for_gpu()
  {
    THROW_IF_FAILED(command_queue->Signal(fence.Get(), fence_value[back_buffer_index]))
    THROW_IF_FAILED(fence->SetEventOnCompletion(fence_value[back_buffer_index], fence_event))
    WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
    fence_value[back_buffer_index]++;
  }

  void fdx12::resize_window(UINT in_width, UINT in_height)
  {
    if(in_width == 0 || in_height == 0)
    {
      return;
    }
    if(in_width == width && in_height == height)
    {
      return;
    }

    wait_for_gpu();
    
    // Release resources
    for (UINT n = 0; n < back_buffer_count; n++)
    {
      rtv[n].Reset();
      fence_value[n] = fence_value[back_buffer_index];
    }

    // Resize
    THROW_IF_FAILED(swap_chain->ResizeBuffers(0, in_width, in_height, DXGI_FORMAT_UNKNOWN, 0))
    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
    
    // Create resources
    {
      D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
      for (UINT n = 0; n < back_buffer_count; n++)
      {
        THROW_IF_FAILED(swap_chain->GetBuffer(n, IID_PPV_ARGS(&rtv[n])))
        device->CreateRenderTargetView(rtv[n].Get(), nullptr, handle);
        handle.ptr += rtv_descriptor_size;
      }
    }
    width = in_width;
    height = in_height;
  }

  void fdx12::cleanup()
  {
    wait_for_gpu();

    for (UINT n = 0; n < back_buffer_count; n++)
    {
      rtv[n].Reset();
      command_allocator[n].Reset();
    }
    DX_RELEASE(swap_chain);
    DX_RELEASE(command_queue);
    DX_RELEASE(command_list);
    DX_RELEASE(rtv_descriptor_heap);
    DX_RELEASE(srv_descriptor_heap);
    DX_RELEASE(fence);
    DX_RELEASE(root_signature);
    DX_RELEASE(device);

    CloseHandle(fence_event);

#ifdef BUILD_DEBUG
    ComPtr<IDXGIDebug1> debug;
    THROW_IF_FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
    DX_RELEASE(debug);
#endif
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
  //void fdx12::create_vertex_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11VertexShader>& out_vertex_shader) const
  //{
  //  THROW_IF_FAILED(device->CreateVertexShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, out_vertex_shader.GetAddressOf()))
  //}
  //
  //void fdx12::create_pixel_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11PixelShader>& out_pixel_shader) const
  //{
  //  THROW_IF_FAILED(device->CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, out_pixel_shader.GetAddressOf()))
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
