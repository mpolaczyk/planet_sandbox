
#include <dxgi1_6.h>
#include "d3d12.h"

#include "core/window.h"

#include <format>

#include "core/application.h"
#include "core/exceptions.h"
#include "renderer/dx12_lib.h"
#include "renderer/renderer_base.h"
#include "renderer/command_queue.h"
#include "renderer/device.h"
#include "hittables/scene.h"
#include "renderer/command_list.h"

namespace engine
{
  fwindow::~fwindow()
  {
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  }

  void fwindow::init(WNDPROC wnd_proc, ComPtr<IDXGIFactory4> factory, const wchar_t* name)
  {
    wc = {sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, name, NULL};
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, name, WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);

    fcommand_queue* command_queue = fapplication::get_instance()->command_queue.get();
    fdevice* device = fapplication::get_instance()->device.get();
    
    screen_tearing = fdx12::enable_screen_tearing(factory);
    fdx12::create_swap_chain(hwnd, factory.Get(), command_queue->com.Get(), back_buffer_count, DXGI_FORMAT_R8G8B8A8_UNORM, screen_tearing, swap_chain);
    
    device->create_render_target_descriptor_heap(rtv_descriptor_heap, "main");
    device->create_depth_stencil_descriptor_heap(dsv_descriptor_heap, "main");
    device->create_cbv_srv_uav_descriptor_heap(main_descriptor_heap, "main");
  }

  void fwindow::show() const
  {
    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);
  }

  void fwindow::hide() const 
  {
    ::ShowWindow(hwnd, SW_HIDE);
  }

  void fwindow::draw()
  {
    fcommand_queue* command_queue = fapplication::get_instance()->command_queue.get();
    fgraphics_command_list* command_list = command_queue->get_command_list(ecommand_list_purpose::main, back_buffer_index);
    
    if(hscene* scene_root = fapplication::get_instance()->scene_root)
    {
      if(rrenderer_base* renderer = scene_root->renderer)
      {
        frenderer_context context;
        context.back_buffer_count = back_buffer_count;
        context.back_buffer_index = back_buffer_index;
        context.scene = scene_root;
        context.selected_object = selected_object;
        context.main_descriptor_heap = &main_descriptor_heap;
        context.rtv_descriptor_heap = &rtv_descriptor_heap;
        context.dsv_descriptor_heap = &dsv_descriptor_heap;
        context.rtv = &rtv[back_buffer_index];
        context.dsv = &dsv;
        context.width = width;
        context.height = height;
        if(renderer->draw(std::move(context), command_list))
        {
          fresource_barrier_scope c(command_list, renderer->get_color()->com.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
          fresource_barrier_scope d(command_list, renderer->get_depth()->com.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE);
          fresource_barrier_scope a(command_list, rtv[back_buffer_index].com.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
          fresource_barrier_scope b(command_list, dsv.com.Get(),                    D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_COPY_DEST);
        
          command_list->com->CopyResource(rtv[back_buffer_index].com.Get(), renderer->get_color()->com.Get());
          command_list->com->CopyResource(dsv.com.Get(), renderer->get_depth()->com.Get());
        }
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

  void fwindow::request_resize(uint32_t in_width, uint32_t in_height)
  {
    requested_width = in_width;
    requested_height = in_height;
  }

  bool fwindow::try_apply_resize()
  {
    if(width == requested_width && height == requested_height)
    {
      return false;
    }
    width = requested_width;
    height = requested_height;
    fcommand_queue* command_queue = fapplication::get_instance()->command_queue.get();
    fdevice* device = fapplication::get_instance()->device.get();

    // Release resources if they already exist
    if(rtv.size() == back_buffer_count)
    {
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        command_queue->flush();
      }
      
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        rtv[n].release();
      }
      rtv.clear();
      dsv.release();
    }
    
    fdx12::resize_swap_chain(swap_chain.Get(), back_buffer_count, width, height);
    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();

    rtv.reserve(back_buffer_count);
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      rtv.emplace_back(ftexture_resource());
      device->create_back_buffer(swap_chain.Get(), n, rtv_descriptor_heap, rtv.back(), std::format("{}",n).c_str());
    }
    device->create_depth_stencil(&dsv_descriptor_heap, &dsv, width, height, DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_STATE_DEPTH_READ, "main");
    return true;
  }

  uint32_t fwindow::get_width() const
  {
    return width == 0 ? requested_width : width;
  }

  uint32_t fwindow::get_height() const
  {
    return height == 0 ? requested_height : height;
  }
}