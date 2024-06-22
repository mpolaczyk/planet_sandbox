

#include <winerror.h>
#include <cassert>

#include "renderer/dx12_lib.h"
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
    if(FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
      throw std::runtime_error("D3D12GetDebugInterface query failed.");
    }
    if(FAILED(debug->QueryInterface(IID_PPV_ARGS(&debug1))))
    {
      throw std::runtime_error("QueryInterface ID3D12Debug1 failed.");
    }
    debug1->SetEnableGPUBasedValidation(true);
    debug->EnableDebugLayer();
#endif

    // Factory, hardware adapter and device
    ComPtr<IDXGIFactory4> factory;
    {
      ComPtr<IDXGIAdapter1> adapter;

      UINT factory_flags = 0;
#if BUILD_DEBUG
      factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
      if(FAILED(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)))) 
      {
        throw std::runtime_error("CreateDXGIFactory2 query failed.");
      }
      
      get_hw_adapter(factory.Get(), &adapter);
      DXGI_ADAPTER_DESC adapter_desc;
      adapter->GetDesc(&adapter_desc);
      LOG_INFO("Graphics Device: {0}", fstring_tools::to_utf8(adapter_desc.Description));
      if(FAILED((D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))))
      {
        throw std::runtime_error("D3D12CreateDevice failed.");
      }
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

    // Commad queue
    {
      D3D12_COMMAND_QUEUE_DESC desc = {};
      desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      if(FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_queue))))
      {
        throw std::runtime_error("CreateCommandQueue failed.");
      }
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

      ComPtr<IDXGISwapChain1> swap_chain1;
      if(FAILED(factory->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &desc, nullptr, nullptr, &swap_chain1)))
      {
        throw std::runtime_error("CreateSwapChainForHwnd failed.");
      }
      if(FAILED(swap_chain1.As(&swap_chain)))
      {
        throw std::runtime_error("IDXGISwapChain3 query failed.");
      }
    }

    // Rtv descriptor heap, descriptor handle, buffer and rtv
    {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.NumDescriptors = back_buffer_count;
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      if(FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtv_descriptor_heap))))
      {
        throw std::runtime_error("CreateDescriptorHeap failed.");
      }
      rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

      D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
      for (UINT n = 0; n < back_buffer_count; n++)
      {
        if(FAILED(swap_chain->GetBuffer(n, IID_PPV_ARGS(&rtv[n]))))
        {
          throw std::runtime_error("RTV GetBuffer failed.");
        }
        device->CreateRenderTargetView(rtv[n].Get(), nullptr, handle);
        //rtv_descriptors[n] = handle;
        handle.ptr += rtv_descriptor_size;
      }
    }

    // Render target shader resource descriptor heap
    {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.NumDescriptors = back_buffer_count;
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
      if(FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srv_descriptor_heap))))
      {
        throw std::runtime_error("CreateDescriptorHeap failed.");
      }
    }

    // Command allocators, and lists
    {
      for (UINT n = 0; n < back_buffer_count; n++)
      {
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocators[n]))))
        {
          throw std::runtime_error("CreateCommandAllocator failed.");
        }
      }
      if(FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocators[0].Get(), nullptr, IID_PPV_ARGS(&command_list))))
      {
        throw std::runtime_error("CreateCommandList failed.");
      }
      if(FAILED(command_list->Close()))
      {
        throw std::runtime_error("Close command list failed.");
      }
    }
    
    // Create an empty root signature.
    {
      CD3DX12_ROOT_SIGNATURE_DESC desc;
      desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      ComPtr<ID3DBlob> signature;
      ComPtr<ID3DBlob> error;
      if(FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)))
      {
        throw std::runtime_error("D3D12SerializeRootSignature failed.");
      }
      if(FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature))))
      {
        throw std::runtime_error("CreateRootSignature failed.");
      }
    }

    // TODO Create pipeline state (shaders, input, etc)
    
    // TODO Create vertes buffer

    // Create synchronization objects
    {
      if(FAILED(device->CreateFence(fence_values[back_buffer_index], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
      {
        throw std::runtime_error("CreateFence failed.");
      }
      fence_values[back_buffer_index]++;

      fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
      if (fence_event == nullptr)
      {
        if(FAILED(HRESULT_FROM_WIN32(GetLastError())))
        {
          throw std::runtime_error("CreateEvent failed.");
        }
      }
      wait_for_gpu();
    }
  }

  void fdx12::wait_for_gpu()
  {
    if(FAILED(command_queue->Signal(fence.Get(), fence_values[back_buffer_index])))
    {
      throw std::runtime_error("Signal failed.");
    }
    if(FAILED(fence->SetEventOnCompletion(fence_values[back_buffer_index], fence_event)))
    {
      throw std::runtime_error("SetEventOnCompletion failed.");
    }
    WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
    fence_values[back_buffer_index]++;
  }
  
  void fdx12::cleanup()
  {


    CloseHandle(fence_event);
  }


  //void fdx12::create_input_layout(const D3D11_INPUT_ELEMENT_DESC* input_element_desc, uint32_t input_element_desc_size, const ComPtr<ID3D10Blob>& vertex_shader_blob, ComPtr<ID3D11InputLayout>& input_layout) const
  //{
  //  if(FAILED(device->CreateInputLayout(input_element_desc, input_element_desc_size, vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), input_layout.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreateInputLayout failed.");
  //  }
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
  //  if(FAILED(device->CreateSamplerState(&desc, &out_sampler_state)))
  //  {
  //    throw std::runtime_error("CreateSamplerState failed.");
  //  }
  //}
  //
  //void fdx12::create_constant_buffer(uint32_t size, ComPtr<ID3D11Buffer>& out_constant_buffer) const
  //{
  //  D3D11_BUFFER_DESC desc = {};
  //  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  //  desc.Usage = D3D11_USAGE_DYNAMIC;
  //  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  //  desc.ByteWidth = size;
  //  if(FAILED(device->CreateBuffer(&desc, nullptr, &out_constant_buffer)))
  //  {
  //    throw std::runtime_error("CreateBuffer failed.");
  //  }
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
  //  if(FAILED(device->CreateRasterizerState(&desc, &out_rasterizer_state)))
  //  {
  //    throw std::runtime_error("CreateRasterizerState failed.");
  //  }
  //}
  //
  //void fdx12::create_depth_stencil_state(ComPtr<ID3D11DepthStencilState>& out_depth_stencil_state) const
  //{
  //  D3D11_DEPTH_STENCIL_DESC desc = {};
  //  desc.DepthEnable = TRUE;
  //  desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  //  desc.DepthFunc = D3D11_COMPARISON_LESS;
  //  desc.StencilEnable = FALSE;
  //  if(FAILED(device->CreateDepthStencilState(&desc, &out_depth_stencil_state)))
  //  {
  //    throw std::runtime_error("CreateDepthStencilState failed.");
  //  }
  //}
  //
  //void fdx12::create_vertex_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11VertexShader>& out_vertex_shader) const
  //{
  //  if(FAILED(device->CreateVertexShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, out_vertex_shader.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreateVertexShader failed");
  //  }
  //}
  //
  //void fdx12::create_pixel_shader(const ComPtr<ID3D10Blob>& shader_blob, ComPtr<ID3D11PixelShader>& out_pixel_shader) const
  //{
  //  if(FAILED(device->CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, out_pixel_shader.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreatePixelShader failed");
  //  }
  //}
  //
  //void fdx12::create_index_buffer(const std::vector<fface_data>& in_face_list, ComPtr<ID3D11Buffer>& out_index_buffer) const
  //{
  //  D3D11_BUFFER_DESC desc = {};
  //  desc.ByteWidth = static_cast<int32_t>(in_face_list.size()) * sizeof(fface_data);
  //  desc.Usage = D3D11_USAGE_IMMUTABLE;
  //  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  //  D3D11_SUBRESOURCE_DATA data = {in_face_list.data()};
  //  if(FAILED(device->CreateBuffer(&desc, &data, out_index_buffer.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreateBuffer index failed.");
  //  }
  //}
  //
  //void fdx12::create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, ComPtr<ID3D11Buffer>& out_vertex_buffer) const
  //{
  //  D3D11_BUFFER_DESC desc = {};
  //  desc.ByteWidth = static_cast<int32_t>(in_vertex_list.size()) * sizeof(fvertex_data);
  //  desc.Usage = D3D11_USAGE_IMMUTABLE;
  //  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  //  const D3D11_SUBRESOURCE_DATA data = {in_vertex_list.data()};
  //  if(FAILED(device->CreateBuffer(&desc, &data, out_vertex_buffer.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreateBuffer vertex failed.");
  //  }
  //}
  //
  //void fdx12::create_render_target_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_RTV_DIMENSION view_dimmension, ComPtr<ID3D11RenderTargetView>& out_render_target_view) const
  //{
  //  D3D11_RENDER_TARGET_VIEW_DESC desc = {};
  //  desc.Format = format;
  //  desc.ViewDimension = view_dimmension;
  //  desc.Texture2D.MipSlice = 0;
  //  if (FAILED(device->CreateRenderTargetView(in_texture.Get(), &desc, out_render_target_view.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreateRenderTargetView failed");
  //  }
  //}
  //
  //void fdx12::create_depth_stencil_view(const ComPtr<ID3D11Texture2D>& in_texture, uint32_t width, uint32_t height, ComPtr<ID3D11DepthStencilView>& out_depth_stencil_view) const
  //{
  //  if(FAILED(device->CreateDepthStencilView(in_texture.Get(), nullptr, out_depth_stencil_view.GetAddressOf()))) // TODO Missing descriptor?
  //  {
  //    throw std::runtime_error("CreateDepthStencilView output depth texture failed");
  //  }
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
  //  if(FAILED(device->CreateTexture2D(&texture_desc, in_bytes != nullptr ? &texture_subresource_data : nullptr, out_texture.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreateTexture2D failed.");
  //  }
  //}
  //
  //void fdx12::create_shader_resource_view(const ComPtr<ID3D11Texture2D>& in_texture, DXGI_FORMAT format, D3D11_SRV_DIMENSION view_dimmension, ComPtr<ID3D11ShaderResourceView>& out_shader_resource_view) const
  //{
  //  D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
  //  shader_resource_view_desc.Format = format;
  //  shader_resource_view_desc.ViewDimension = view_dimmension;
  //  shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
  //  shader_resource_view_desc.Texture2D.MipLevels = 1;
  //  if(FAILED(device->CreateShaderResourceView(in_texture.Get(), &shader_resource_view_desc, out_shader_resource_view.GetAddressOf())))
  //  {
  //    throw std::runtime_error("CreateShaderResourceView failed.");
  //  }
  //}
}
