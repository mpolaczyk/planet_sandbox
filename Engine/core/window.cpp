
#include <dxgi1_6.h>
#include "d3d12.h"

#include "core/window.h"

#include <format>

#include "core/application.h"
#include "core/exceptions.h"
#include "renderer/dx12_lib.h"
#include "renderer/renderer_base.h"
#include "renderer/command_queue.h"
#include "hittables/scene.h"
#include "renderer/scene_acceleration.h"

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

  void fwindow::init(WNDPROC wnd_proc, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue)
  {
    fdevice& device = fapplication::instance->device;
    
    wc = {sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, get_name(), NULL};
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, get_name(), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    
    screen_tearing = fdx12::enable_screen_tearing(factory);

    fdx12::create_swap_chain(hwnd, factory, command_queue, back_buffer_count, screen_tearing, swap_chain);
    device.create_render_target_descriptor_heap(back_buffer_count, rtv_descriptor_heap);
    device.create_depth_stencil_descriptor_heap(dsv_descriptor_heap);
    device.create_cbv_srv_uav_descriptor_heap(main_descriptor_heap);
#if BUILD_DEBUG
    DX_SET_NAME(rtv_descriptor_heap, "Render target descriptor heap")
    DX_SET_NAME(dsv_descriptor_heap, "Depth stencil descriptor heap")
    DX_SET_NAME(main_descriptor_heap.heap, "Main descriptor heap")
#endif
  }

  void fwindow::draw(const fcommand_queue* command_queue)
  {
    ComPtr<ID3D12GraphicsCommandList> command_list = command_queue->get_command_list(ecommand_list_type::main, back_buffer_index);
    fdevice& device = fapplication::instance->device;
    
    if(hscene* scene_root = fapplication::instance->scene_root)
    {
      if(rrenderer_base* renderer = scene_root->renderer)
      {
        fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        
        fdx12::clear_render_target(device.device, command_list, rtv_descriptor_heap, back_buffer_index);
        fdx12::clear_depth_stencil(command_list, dsv_descriptor_heap);
        fdx12::set_render_targets(device.device, command_list, dsv_descriptor_heap, rtv_descriptor_heap, back_buffer_index);
        fdx12::set_viewport(command_list, width, height);
        fdx12::set_scissor(command_list, width, height);
        
        frenderer_context context;
        context.back_buffer_count = back_buffer_count;
        context.back_buffer_index = back_buffer_index;
        context.rtv = rtv[back_buffer_index];
        context.dsv = dsv;
        context.scene = scene_root;
        context.selected_object = selected_object;
        context.main_descriptor_heap = &main_descriptor_heap;
        renderer->set_renderer_context(std::move(context));
        renderer->output_width = renderer->output_width == 0 ? width : renderer->output_width;
        renderer->output_height = renderer->output_height == 0 ? height : renderer->output_height;
        renderer->draw(command_list);

        fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
      }
    }
  }
  
  void fwindow::present(fgpu_crash_tracker* gpu_crash_handler)
  {
    uint32_t present_sync = vsync ? 1 : 0;
    uint32_t present_flags = screen_tearing && !vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

#if USE_NSIGHT_AFTERMATH
    HRESULT hr = swap_chain->Present(present_sync, present_flags);
    if(hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_DEVICE_REMOVED)
    {
      gpu_crash_handler->wait_for_dump_and_throw(hr);
    }
    THROW_IF_FAILED(hr)
    gpu_crash_handler->advance_frame();
#else
    THROW_IF_FAILED(swap_chain->Present(present_sync, present_flags))
#endif
    
    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
  }
  
  void fwindow::resize(uint32_t in_width, uint32_t in_height)
  {
    fdevice& device = fapplication::instance->device;
    
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
      DX_RELEASE(dsv);
    }

    fdx12::resize_swap_chain(swap_chain, back_buffer_count, width, height);
    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
    device.create_render_target(swap_chain.Get(), rtv_descriptor_heap.Get(), back_buffer_count, rtv);
    device.create_depth_stencil(dsv_descriptor_heap.Get(), width, height, dsv);
#if BUILD_DEBUG
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      DX_SET_NAME(rtv[n], "Render target resource: {}", n)
    }
    DX_SET_NAME(dsv, "Depth stencil resource")
#endif
  }
  
  void fwindow::cleanup()
  {
    DX_RELEASE(swap_chain);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      DX_RELEASE(rtv[n]);
    }
    DX_RELEASE(dsv);
    DX_RELEASE(main_descriptor_heap.heap);
    DX_RELEASE(rtv_descriptor_heap);
    DX_RELEASE(dsv_descriptor_heap);

    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  }
}