#include <dxgi1_5.h>

#include "core/application.h"
#include "core/exceptions.h"
#include "core/window.h"
#include "engine/io.h"
#include "object/object_registry.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "math/random.h"
#include "renderer/dx12_lib.h"
#include "renderer/command_queue.h"
#include "renderer/renderer_base.h"
#include "resources/assimp_logger.h"

namespace engine
{
  fapplication* fapplication::instance = nullptr;
  ftimer_instance fapplication::stat_frame_time;
  ftimer_instance fapplication::stat_update_time;
  ftimer_instance fapplication::stat_draw_time;
  ftimer_instance fapplication::stat_render_time;
  uint64_t fapplication::frame_counter;
  
  LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return fapplication::instance->wnd_proc(hWnd, msg, wParam, lParam);
  }

  LRESULT fapplication::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    switch(msg)
    {
    case WM_SIZE:
      if(device.com != nullptr && wParam != SIZE_MINIMIZED)
      {
        window->resize(LOWORD(lParam), HIWORD(lParam));
      }
      return 0;
    case WM_SYSCOMMAND:
      if((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
        return 0;
      break;
    case WM_DESTROY:
      ::PostQuitMessage(0);
      return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
  }

  void fapplication::init(const char* project_name)
  {
    fio::init(project_name);
    flogger::init();
    fassimp_logger::init();
    frandom_cache::init();

    REG.create_class_objects();

    LOG_INFO("Project: {0}", fio::get_project_name());
    LOG_INFO("Working dir: {0}", fio::get_working_dir());
    LOG_INFO("Workspace dir: {0}", fio::get_workspace_dir());
    LOG_INFO("Project dir: {0}", fio::get_project_dir());
    if(!fio::validate_workspace_dir())
    {
      LOG_CRITICAL("Invalid workspace directory!");
    }

    ComPtr<IDXGIFactory4> factory;
    fdx12::create_factory(factory);
    
#if USE_NSIGHT_AFTERMATH
    gpu_crash_handler.pre_device_creation(window->back_buffer_count);
#endif
    
#if BUILD_DEBUG && !USE_NSIGHT_AFTERMATH
    fdx12::enable_debug_layer();
#else
    LOG_WARN("Disabling the DX12 debug layer and GPU validation!")
#endif
    
    device = fdevice::create(factory.Get());

#if USE_NSIGHT_AFTERMATH
    gpu_crash_handler.post_device_creation(device.com.Get());
#endif
    
#if BUILD_DEBUG && !USE_NSIGHT_AFTERMATH
    device.enable_info_queue();
#else
    LOG_WARN("Disabling the info queue!")
#endif
    
    command_queue = std::make_shared<fcommand_queue>();
    command_queue->init(device, window->back_buffer_count);
    
#if USE_NSIGHT_AFTERMATH
    for (uint32_t i = 0; i < window->back_buffer_count; i++)
    {
      // Requires command list to be in a recording state, otherwise it crashes with 0xbad00000
      std::shared_ptr<fgraphics_command_list> command_list = command_queue->get_command_list(ecommand_list_purpose::main, i);
      gpu_crash_handler.create_context_handle(i, command_list->com.Get());
    }
#endif

    command_queue->flush();
    
    scene_root = hscene::spawn();
    
    window->init(WndProc, factory);
    window->show();

    LOG_INFO("Init done, starting the main loop");
  }

  void fapplication::main_loop()
  {
    while(is_running)
    {
      fscope_timer frame_timer(stat_frame_time);
      MSG msg;
      while(::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
      {
        ::TranslateMessage(&msg);
        if(msg.message == WM_QUIT)
        {
          is_running = false;
        }
        ::DispatchMessage(&msg);
      }
      if(!is_running) break;
      
      {
        fscope_timer update_timer(stat_update_time);
        update();
      }
      {
        fscope_timer draw_timer(stat_draw_time);
        draw();
      }
      {
        fscope_timer render_timer(stat_render_time);
        render();
      }
      flogger::flush();
    }
  }

  void fapplication::update()
  {
    if(scene_root != nullptr && scene_root->renderer != nullptr)
    {
      scene_root->camera_config.update(stat_frame_time.get_last_time_ms(), scene_root->renderer->output_width, scene_root->renderer->output_height);
    }
    window->update();
  }

  void fapplication::draw()
  {
    window->draw();
  }

  void fapplication::render()
  {
    const int back_buffer_index = window->get_back_buffer_index();

    command_queue->close_command_lists();
    uint64_t fence_value = command_queue->execute_command_lists(back_buffer_index);
    command_queue->reset_command_lists(back_buffer_index);
#if USE_NSIGHT_AFTERMATH
    window->present(&gpu_crash_handler);
#else
    window->present(nullptr);
#endif
    command_queue->wait_for_fence_value(fence_value);   // TODO CPU waits for GPU, make it async
    frame_counter++;
  }
  
  void fapplication::cleanup()
  {
    if (scene_root)
    {
      scene_root->destroy();
    }
    command_queue->flush();
    command_queue->flush();
    command_queue->flush();
    command_queue->flush();
    command_queue->cleanup();
    window->cleanup();
#ifdef BUILD_DEBUG
    fdx12::report_live_objects();
#endif
    flogger::flush();
  }
}