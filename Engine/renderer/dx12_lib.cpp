#include <winerror.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#pragma comment(lib, "dxguid.lib")
#include "d3d12.h"
#include "d3dx12/d3dx12_root_signature.h"

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

    if(SUCCEEDED(in_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
      for(uint32_t adapter_index = 0;
          SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapter_index,prefer_high_performance_adapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter1)));
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
      for(uint32_t adapter_index = 0; SUCCEEDED(in_factory->EnumAdapters1(adapter_index, &adapter1)); ++adapter_index)
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

  void fdx12::enable_debug_layer()
  {
    ComPtr<ID3D12Debug> debug;
    ComPtr<ID3D12Debug1> debug1;
    THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))
    THROW_IF_FAILED(debug->QueryInterface(IID_PPV_ARGS(&debug1)))
    debug1->SetEnableGPUBasedValidation(true);
    debug->EnableDebugLayer();
  }

  void fdx12::create_factory(ComPtr<IDXGIFactory4>& out_factory4)
  {
    uint32_t factory_flags = 0;
#if BUILD_DEBUG
    factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    THROW_IF_FAILED(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&out_factory4)))
  }

  void fdx12::create_device(const ComPtr<IDXGIFactory4>& in_factory, ComPtr<ID3D12Device>& out_device)
  {
    ComPtr<IDXGIAdapter1> adapter1;
    get_hw_adapter(in_factory.Get(), &adapter1);
    DXGI_ADAPTER_DESC adapter_desc;
    adapter1->GetDesc(&adapter_desc);
    LOG_INFO("Graphics Device: {0}", fstring_tools::to_utf8(adapter_desc.Description));
    THROW_IF_FAILED((D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&out_device))))
  }

  bool fdx12::enable_screen_tearing(const ComPtr<IDXGIFactory4>& in_factory)
  {
    bool success = false;
    ComPtr<IDXGIFactory5> factory5;
    if(SUCCEEDED(in_factory.As(&factory5)))
    {
      if(FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &success, sizeof(success))))
      {
        success = false;
      }
    }
    return success;
  }

  void fdx12::enable_info_queue(const ComPtr<ID3D12Device>& in_device)
  {
    ID3D12InfoQueue* info_queue = nullptr;
    in_device->QueryInterface(IID_PPV_ARGS(&info_queue));
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    info_queue->Release();
  }

  void fdx12::create_command_queue(const ComPtr<ID3D12Device>& in_device, ComPtr<ID3D12CommandQueue>& out_command_queue)
  {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    THROW_IF_FAILED(in_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&out_command_queue)))
  }

  void fdx12::create_swap_chain(HWND hwnd, const ComPtr<IDXGIFactory4>& in_factory, const ComPtr<ID3D12CommandQueue>& in_command_queue, int back_buffer_count, bool allow_screen_tearing, ComPtr<IDXGISwapChain3>& out_swap_chain)
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
    THROW_IF_FAILED(in_factory->CreateSwapChainForHwnd(in_command_queue.Get(), hwnd, &desc, nullptr, nullptr, &swap_chain1))
    THROW_IF_FAILED(in_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    THROW_IF_FAILED(swap_chain1.As(&out_swap_chain))
  }

  void fdx12::create_render_target(const ComPtr<ID3D12Device>& in_device, const ComPtr<IDXGISwapChain3>& in_swap_chain, int back_buffer_count, ComPtr<ID3D12DescriptorHeap>& out_rtv_descriptor_heap, uint32_t& out_rtv_descriptor_size, std::vector<ComPtr<ID3D12Resource>>& out_resource)
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = back_buffer_count;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    THROW_IF_FAILED(in_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&out_rtv_descriptor_heap)))

    out_rtv_descriptor_size = in_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(out_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());
    out_resource.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      out_resource.push_back(nullptr);
      THROW_IF_FAILED(in_swap_chain->GetBuffer(n, IID_PPV_ARGS(&out_resource[n])))
      in_device->CreateRenderTargetView(out_resource[n].Get(), nullptr, handle);
      handle.Offset(static_cast<int>(out_rtv_descriptor_size));
    }
  }

  void fdx12::create_shader_resource(const ComPtr<ID3D12Device>& in_device, ComPtr<ID3D12DescriptorHeap>& out_srv_descriptor_heap)
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    THROW_IF_FAILED(in_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&out_srv_descriptor_heap)))
  }

  void fdx12::create_command_list(const ComPtr<ID3D12Device>& in_device, int back_buffer_count, ComPtr<ID3D12GraphicsCommandList>& out_command_list, std::vector<ComPtr<ID3D12CommandAllocator>>& out_command_allocators)
  {
    out_command_allocators.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      out_command_allocators.push_back(nullptr);
      THROW_IF_FAILED(in_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&out_command_allocators[n])))
    }
    THROW_IF_FAILED(in_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, out_command_allocators[0].Get(), nullptr, IID_PPV_ARGS(&out_command_list)))
    THROW_IF_FAILED(out_command_list->Close())
  }

  void fdx12::create_root_signature(const ComPtr<ID3D12Device>& in_device, ComPtr<ID3D12RootSignature>& out_root_signature)
  {
    CD3DX12_ROOT_SIGNATURE_DESC desc;
    desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    THROW_IF_FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))
    THROW_IF_FAILED(in_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&out_root_signature)))
  }

  void fdx12::create_synchronisation(const ComPtr<ID3D12Device>& in_device, int back_buffer_count, int initial_fence_value, ComPtr<ID3D12Fence>& out_fence, HANDLE& out_fence_event, std::vector<uint64_t>& out_fence_values)
  {
    THROW_IF_FAILED(in_device->CreateFence(initial_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&out_fence)))

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

  void fdx12::report_live_objects()
  {
    ComPtr<IDXGIDebug1> debug;
    THROW_IF_FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
    DX_RELEASE(debug);
  }
  
  void fdx12::create_vertex_buffer(const std::vector<fvertex_data>& in_vertex_list, fstatic_mesh_render_state& out_render_state)
  {
    //UINT num_vertices = in_vertex_list.size();
    //{
    //  const CD3DX12_HEAP_PROPERTIES heap_props{D3D12_HEAP_TYPE_DEFAULT};
    //  const auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(in_vertex_list));
    //  THROW_IF_FAILED(device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&out_render_state.vertex_buffer)));
    //}
    //// create committed resource for cpu upload of vertex data 
    //ComPtr<ID3D12Resource> vertex_upload_buffer;
    //{
    //  const CD3DX12_HEAP_PROPERTIES heap_props{D3D12_HEAP_TYPE_UPLOAD};
    //  const auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(in_vertex_list));
    //  THROW_IF_FAILED(device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_upload_buffer)));
    //}
    //// copy array of vertex data to upload buffer 
    //{
    //  fvertex_data* mapped_vertex_data = nullptr;
    //  CD3DX12_RANGE read_range(0, 0);
    //  THROW_IF_FAILED(vertex_upload_buffer->Map(0, &read_range, reinterpret_cast<void**>(&mapped_vertex_data)));
    //  memcpy(mapped_vertex_data, in_vertex_list.data(), sizeof(fvertex_data));
    //  vertex_upload_buffer->Unmap(0, nullptr);
    //}
    //// reset command list and allocator  
    //THROW_IF_FAILED(command_allocator[back_buffer_index]->Reset());
    //THROW_IF_FAILED(command_list->Reset(command_allocator[back_buffer_index].Get(), nullptr));
    //// copy upload buffer to vertex buffer 
    //command_list->CopyResource(out_render_state.vertex_buffer.Get(), vertex_upload_buffer.Get());
    //// transition vertex buffer to vertex buffer state 
    //{
    //  const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(out_render_state.vertex_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    //  command_list->ResourceBarrier(1, &barrier);
    //}
    //// close command list  
    //THROW_IF_FAILED(command_list->Close());
    //// submit command list to queue as array with single element 
    //ID3D12CommandList* const command_lists[] = {command_list.Get()};
    //command_queue->ExecuteCommandLists((UINT)std::size(command_lists), command_lists);
    //// insert fence to detect when upload is complete
//
    //flush(last_fence_value);
//
    //// Create the vertex buffer view. 
    //out_render_state.vertex_buffer_view.BufferLocation = out_render_state.vertex_buffer->GetGPUVirtualAddress();
    //out_render_state.vertex_buffer_view.SizeInBytes = num_vertices * sizeof(fvertex_data);
    //out_render_state.vertex_buffer_view.StrideInBytes = sizeof(fvertex_data);
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