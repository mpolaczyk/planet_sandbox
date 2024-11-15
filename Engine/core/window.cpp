
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

    std::shared_ptr<fcommand_queue> command_queue = fapplication::instance->command_queue;
    fdevice& device = fapplication::instance->device;
    
    screen_tearing = fdx12::enable_screen_tearing(factory);
    fdx12::create_swap_chain(hwnd, factory.Get(), command_queue->com.Get(), back_buffer_count, screen_tearing, swap_chain);
    
    device.create_render_target_descriptor_heap(back_buffer_count, rtv_descriptor_heap, "main");
    device.create_depth_stencil_descriptor_heap(dsv_descriptor_heap, "main");
    device.create_cbv_srv_uav_descriptor_heap(main_descriptor_heap, "main");
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
    std::shared_ptr<fcommand_queue> command_queue = fapplication::instance->command_queue;
    std::shared_ptr<fgraphics_command_list> command_list = command_queue->get_command_list(ecommand_list_purpose::main, back_buffer_index);
    
    if(hscene* scene_root = fapplication::instance->scene_root)
    {
      if(rrenderer_base* renderer = scene_root->renderer)
      {
        command_list->resource_barrier(rtv[back_buffer_index].resource.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->clear_render_target(rtv[back_buffer_index]);
        command_list->clear_depth_stencil(dsv);
        command_list->set_render_targets(rtv[back_buffer_index], dsv);
        command_list->set_viewport(width, height);
        command_list->set_scissor(width, height);
        
        frenderer_context context;
        context.back_buffer_count = back_buffer_count;
        context.back_buffer_index = back_buffer_index;
        context.rtv = &rtv[back_buffer_index];
        context.dsv = &dsv;
        context.scene = scene_root;
        context.selected_object = selected_object;
        context.main_descriptor_heap = &main_descriptor_heap;
        renderer->set_renderer_context(std::move(context));
        renderer->output_width = renderer->output_width == 0 ? width : renderer->output_width;
        renderer->output_height = renderer->output_height == 0 ? height : renderer->output_height;
        renderer->draw(command_list.get());

        command_list->resource_barrier(rtv[back_buffer_index].resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
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

  bool fwindow::apply_resize()
  {
    if(width == requested_width && height == requested_height)
    {
      return false;
    }
    width = requested_width;
    height = requested_height;
    std::shared_ptr<fcommand_queue> command_queue = fapplication::instance->command_queue;
    fdevice& device = fapplication::instance->device;

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
      frtv_resource temp;
      device.create_render_target(swap_chain.Get(), n, rtv_descriptor_heap, temp, std::format("back buffer {}",n).c_str());
      rtv.emplace_back(std::move(temp));
    }
    device.create_depth_stencil(dsv_descriptor_heap, width, height, dsv, "main");
    return true;
  }
}