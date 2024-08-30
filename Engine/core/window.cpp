
#include <dxgi1_6.h>
#include "d3d12.h"
#include "d3dx12/d3dx12_root_signature.h"

#include "core/window.h"

#include <format>

#include "core/application.h"
#include "core/exceptions.h"
#include "renderer/dx12_lib.h"
#include "renderer/renderer_base.h"
#include "renderer/command_queue.h"
#include "hittables/scene.h"

namespace engine
{
  void fwindow::show() const
  {
    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);
  }

  void fwindow::hide() const 
  {
    ::ShowWindow(hwnd, SW_HIDE);
  }

  void fwindow::init(WNDPROC wnd_proc, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue)
  {
    wc = {sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, get_name(), NULL};
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, get_name(), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    
    screen_tearing = fdx12::enable_screen_tearing(factory);

    fdx12::create_swap_chain(hwnd, factory, command_queue, back_buffer_count, screen_tearing, swap_chain);
    fdx12::create_render_target_descriptor_heap(device, back_buffer_count, rtv_descriptor_heap);
    fdx12::create_depth_stencil_descriptor_heap(device, dsv_descriptor_heap);
    fdx12::create_cbv_srv_uav_descriptor_heap(device, main_descriptor_heap);
#if BUILD_DEBUG
    rtv_descriptor_heap->SetName(L"Render target descriptor heap");
    dsv_descriptor_heap->SetName(L"Depth stencil descriptor heap");
    main_descriptor_heap->SetName(L"Main descriptor heap");
#endif
    
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      ComPtr<ID3D12Resource> resource;
      fdx12::create_upload_resource(device, 1024, resource);
      cbv.push_back(resource);
#if BUILD_DEBUG
      std::string name = std::format("Constant buffer upload resource: back buffer {}", n);
      resource->SetName(std::wstring(name.begin(), name.end()).c_str());
#endif
    }
  }

  void fwindow::render(const fcommand_queue* command_queue)
  {
    ComPtr<ID3D12GraphicsCommandList> command_list = command_queue->get_command_list(ecommand_list_type::main, back_buffer_index);
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    fdx12::clear_render_target(device, command_list, rtv_descriptor_heap, back_buffer_index);
    fdx12::clear_depth_stencil(command_list, dsv_descriptor_heap);
    
    fdx12::set_render_targets(device, command_list, dsv_descriptor_heap, rtv_descriptor_heap, back_buffer_index);
    fdx12::set_viewport(command_list, width, height);
    fdx12::set_scissor(command_list, width, height);
    
    command_list->SetDescriptorHeaps(1, main_descriptor_heap.GetAddressOf());

    if(hscene* scene_root = fapplication::instance->scene_root)
    {
      if(rrenderer_base* renderer = scene_root->renderer)
      {
        renderer->render_frame(command_list, this, scene_root, nullptr);  // TODO selected object
      }
    }

    fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  }
  
  void fwindow::present()
  {
    uint32_t present_sync = vsync ? 1 : 0;
    uint32_t present_flags = screen_tearing && !vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    THROW_IF_FAILED(swap_chain->Present(present_sync, present_flags))

    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
  }
  
  void fwindow::resize(const ComPtr<ID3D12Device> device, uint32_t in_width, uint32_t in_height)
  {
    if(in_width == 0 || in_height == 0) { return; }
    if(in_width == width && in_height == height) { return; }
    
    width = in_width;
    height = in_height;

    if(rtv.size() == back_buffer_count)
    {
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        DX_RELEASE(rtv[n]);
      }
      rtv.clear();
    }
    DX_RELEASE(dsv);

    fdx12::resize_swap_chain(swap_chain, back_buffer_count, width, height);
    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
    fdx12::create_render_target(device, swap_chain, rtv_descriptor_heap, back_buffer_count, rtv);
    fdx12::create_depth_stencil(device, dsv_descriptor_heap, width, height, dsv);
#if BUILD_DEBUG
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      std::string rtv_name = std::format("Render target resource: back buffer {}", n);
      rtv[n]->SetName(std::wstring(rtv_name.begin(), rtv_name.end()).c_str());
    }
    dsv->SetName(L"Depth stencil resource");
#endif
  }
  
  void fwindow::cleanup()
  {
    DX_RELEASE(swap_chain);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      DX_RELEASE(rtv[n]);
      DX_RELEASE(cbv[n])
    }
    DX_RELEASE(main_descriptor_heap);
    DX_RELEASE(rtv_descriptor_heap);
    DX_RELEASE(dsv);
    DX_RELEASE(dsv_descriptor_heap);

    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  }
}