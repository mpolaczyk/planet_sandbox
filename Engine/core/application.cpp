#include "stdafx.h"

#include "assets/shader.h"

#include "hittables/scene.h"

#include "engine/window.h"
#include "engine/io.h"
#include "engine/math/random.h"
#include "engine/physics.h"
#include "engine/persistence/object_persistence.h"
#include "engine/persistence/persistence_helper.h"
#include "engine/renderer/dx12_lib.h"
#include "engine/renderer/command_queue.h"
#include "engine/renderer/renderer_base.h"
#include "engine/renderer/device.h"
#include "engine/resources/assimp_logger.h"

namespace engine
{
  fapplication* fapplication::instance = nullptr; // weak ptr, no ownership
  ftimer_instance fapplication::stat_frame_time;
  ftimer_instance fapplication::stat_update_time;
  ftimer_instance fapplication::stat_draw_time;
  ftimer_instance fapplication::stat_render_time;
  uint64_t fapplication::frame_counter;
  
  LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return fapplication::get_instance()->wnd_proc(hWnd, msg, wParam, lParam);
  }

  fapplication::fapplication() = default; // Because fshared_ptr<forward declared type> requires destructor where the type is complete

  fapplication::~fapplication()
  {
    // The rest could be destroyed automatically, but I want to keep order and logging
    LOG_INFO("Destroying physics scene");
    physics.reset();
    
    LOG_INFO("Destroying managed objects");
    REG.destroy_all();

    LOG_INFO("Destroying other resources");
    command_queue.reset();
    window.reset();
    
    flogger::flush();
  }

  LRESULT fapplication::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    switch(msg)
    {
    case WM_SIZE:
      if(device->com.Get() != nullptr && wParam != SIZE_MINIMIZED)
      {
        window->request_resize(LOWORD(lParam), HIWORD(lParam));
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
    LOG_TRACE("Application init");
    fio::init(project_name);
    fassimp_logger::init();
    frandom_cache::init();

    LOG_INFO("Project: {0}", fio::get_project_name());
    LOG_INFO("Working dir: {0}", fio::get_working_dir());
    LOG_INFO("Workspace dir: {0}", fio::get_workspace_dir());
    LOG_INFO("Project dir: {0}", fio::get_project_dir());
    if(!fio::validate_workspace_dir())
    {
      LOG_CRITICAL("Invalid workspace directory!");
    }
    
    LOG_INFO("Creating class objects");
    REG.create_class_objects();

    LOG_INFO("Creating rendering resources");
    fcom_ptr<IDXGIFactory4> factory;
    fdx12::create_factory(factory);
#if USE_NSIGHT_AFTERMATH
    gpu_crash_handler.pre_device_creation(window->back_buffer_count);
#endif
#if BUILD_DEBUG && !USE_NSIGHT_AFTERMATH && !USE_NSIGHT_GRAPHICS
    fdx12::enable_debug_layer_and_gpu_validation();
#else
    LOG_WARN("Disabling the DX12 debug layer and GPU validation!")
#endif
    device.reset(fdevice::create(factory.Get()));
#if USE_NSIGHT_AFTERMATH
    gpu_crash_handler.post_device_creation(device.get()->com.Get());
#endif
#if BUILD_DEBUG && !USE_NSIGHT_AFTERMATH && !USE_NSIGHT_GRAPHICS
    device->enable_info_queue();
#else
    LOG_WARN("Disabling the info queue!")
#endif
    command_queue.reset(new fcommand_queue(device.get(), window->back_buffer_count));
#if USE_NSIGHT_AFTERMATH
    for (uint32_t i = 0; i < window->back_buffer_count; i++)
    {
      // Requires command list to be in a recording state, otherwise it crashes with 0xbad00000
      fgraphics_command_list* command_list = command_queue->get_command_list(ecommand_list_purpose::main, i);
      gpu_crash_handler.create_context_handle(i, command_list->com.Get());
    }
#endif
    command_queue->flush();

    LOG_INFO("Loading scene persistent state");
    scene_root = hscene::spawn();
    load_scene_state();
    
    LOG_INFO("Creating physics scene");
    physics.reset(new fphysics);
    physics->create_physics(scene_root);

    LOG_INFO("Initiating the window");
    window->init(WndProc, factory.Get(), L"Editor");
    window->show();
    
    LOG_INFO("Application init done, starting the main loop");
  }

  void fapplication::set_window(fwindow* in_window)
  {
    window.reset(in_window);
  }

  void fapplication::main_loop()
  {
    while(is_running)
    {
      g_frame_number++;
      g_frame_time_ms = stat_frame_time.get_last_time_ms();
      float delta_time = g_frame_time_ms / 1000.0f;
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
        update(delta_time);
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

  void fapplication::update(float delta_time)
  {
    try_hot_swap_shaders();     // TODO does not have to happen every frame
    
    // Generic update
    if(scene_root != nullptr && scene_root->renderer != nullptr)
    {
      physics->update_physics(delta_time);  // TODO Fixed time step :) ?
      scene_root->camera.update(delta_time, window->get_width(), window->get_height());
      window->update();
    }
  }

  void fapplication::draw()
  {
    if(!window->try_apply_resize() && fapplication::frame_counter != 0)
    {
      command_queue->reset_allocator(window->get_back_buffer_index());
    }
    window->draw();
  }

  void fapplication::render()
  {
    const uint64_t fence_value = command_queue->execute_command_lists(window->get_back_buffer_index());
#if USE_NSIGHT_AFTERMATH
    window->present(&gpu_crash_handler);
#else
    window->present(nullptr);
#endif
    command_queue->wait_for_fence_value(fence_value);   // TODO CPU waits for GPU, make it async
    frame_counter++;
  }

  void fapplication::load_scene_state() const // TODO move hscene::load(), btw. scene should be an asset
  {
    LOG_INFO("Loading: scene");

    std::string path = fio::get_scene_file_path();
    std::ifstream input_stream(path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open file: {0}", path);
      return;
    }
    nlohmann::json j;
    input_stream >> j;

    nlohmann::json jscene_root;
    if (TRY_PARSE(nlohmann::json, j, "scene", jscene_root)) { scene_root->accept(vdeserialize_object(jscene_root)); }

    input_stream.close();
  }

  void fapplication::save_scene_state() const // TODO move to hscene::save() , btw. scene should be an asset
  {
    LOG_INFO("Saving: scene");

    nlohmann::json j;
    scene_root->accept(vserialize_object(j["scene"]));
    std::ofstream o(fio::get_scene_file_path().c_str(), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    o.close();
  }

  void fapplication::try_hot_swap_shaders()
  {
    const std::vector<ashader*> shader_assets = REG.get_all_by_type<ashader>();
    std::vector<ashader*> shader_assets_to_hot_swap;
    for(ashader* asset : shader_assets)
    {
      const std::string file_path = fio::get_shader_file_path(asset->shader_file_name.c_str());
      const int64_t timestamp = fio::get_last_write_time(file_path.c_str());
      if(asset->hot_swap_done)
      {
        asset->hot_swap_requested = false;
        asset->hot_swap_done = false;
        continue;
      }
      if(timestamp != asset->hlsl_file_timestamp)
      {
        shader_assets_to_hot_swap.push_back(asset);
      }
    }
    for(ashader* asset : shader_assets_to_hot_swap)
    {
      LOG_INFO("Detected HLSL changes in shader: {}", asset->get_display_name())
      if(asset->load(asset->name))
      {
        asset->hot_swap_requested = true;
      }
    }
  }

  void fapplication::log_heap_descriptors() const
  {
    if(window)
    {
      window->rtv_descriptor_heap.log_audit();
      window->dsv_descriptor_heap.log_audit();
      window->main_descriptor_heap.log_audit();
    }
  }
}