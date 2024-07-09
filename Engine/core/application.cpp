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

  LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return fapplication::instance->wnd_proc(hWnd, msg, wParam, lParam);
  }

  LRESULT fapplication::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    switch(msg)
    {
    case WM_SIZE:
      if(device != nullptr && wParam != SIZE_MINIMIZED)
      {
        command_queue->flush();
        window->resize(device, LOWORD(lParam), HIWORD(lParam));
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

#if BUILD_DEBUG
    fdx12::enable_debug_layer();
#endif
    ComPtr<IDXGIFactory4> factory;
    fdx12::create_factory(factory);
    fdx12::create_device(factory, device);
#if BUILD_DEBUG
    fdx12::enable_info_queue(device);
#endif

    command_queue = std::make_shared<fcommand_queue>();
    command_queue->init(device, window->back_buffer_count);
    command_queue->flush();

    scene_root = hscene::spawn();
    
    window->init(WndProc, device, factory, command_queue->get_command_queue());
    window->show();

    LOG_INFO("Init done, starting the main loop");
  }

  void fapplication::main_loop()
  {
    while(is_running)
    {
      MSG msg;
      while(::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
      {
        if(msg.message == WM_CLOSE)
        {
          is_running = false;
        }
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
      }
      if(!is_running) break;

      update();
      window->update();
      window->draw();

      ComPtr<ID3D12GraphicsCommandList> command_list = command_queue->get_command_list(window->get_back_buffer_index());
      window->render(command_list);
      uint64_t fence_value = command_queue->execute_command_list(window->get_back_buffer_index());
      window->present();
      command_queue->wait_for_fence_value(fence_value);
    }
  }

  void fapplication::update()
  {
    //const ImGuiIO& io = ImGui::GetIO();
    //app_delta_time_ms = io.DeltaTime * 1000.0f;
    render_delta_time_ms = static_cast<float>(scene_root->renderer->get_render_time_ms());

    if(scene_root != nullptr && scene_root->renderer != nullptr)
    {
      scene_root->camera_config.update(render_delta_time_ms / 1000.0f, scene_root->renderer->output_width, scene_root->renderer->output_height);
    }
  }
  
  void fapplication::cleanup()
  {
    if (scene_root)
    {
      scene_root->destroy();
    }
    command_queue->flush();
    command_queue->cleanup();
    window->cleanup();
    DX_RELEASE(device);
#ifdef BUILD_DEBUG
    fdx12::report_live_objects();
#endif
  }
}