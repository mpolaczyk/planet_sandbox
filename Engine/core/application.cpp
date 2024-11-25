#include "core/application.h"

#include <dxgi1_5.h>

#include "core/exceptions.h"
#include "core/window.h"
#include "engine/io.h"
#include "object/object_registry.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "math/random.h"
#include "persistence/object_persistence.h"
#include "persistence/persistence_helper.h"
#include "renderer/dx12_lib.h"
#include "renderer/command_queue.h"
#include "renderer/renderer_base.h"
#include "renderer/device.h"
#include "resources/assimp_logger.h"

#include "nlohmann/json.hpp"
#include "reactphysics3d/engine/PhysicsCommon.h"
#include "reactphysics3d/engine/PhysicsWorld.h"

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

  fapplication::~fapplication()
  {
    LOG_INFO("Destroying managed objects");
    REG.destroy_all();

    if(physics_world)
    {
      LOG_INFO("Destroying physics scene");
      end_physics();
    }
    
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
      if(device->com != nullptr && wParam != SIZE_MINIMIZED)
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
    device.reset(fdevice::create(factory.Get()));
#if USE_NSIGHT_AFTERMATH
    gpu_crash_handler.post_device_creation(device.com.Get());
#endif
#if BUILD_DEBUG && !USE_NSIGHT_AFTERMATH
    device->enable_info_queue();
#else
    LOG_WARN("Disabling the info queue!")
#endif
    command_queue.reset(new fcommand_queue(device.get(), window->back_buffer_count));
#if USE_NSIGHT_AFTERMATH
    for (uint32_t i = 0; i < window->back_buffer_count; i++)
    {
      // Requires command list to be in a recording state, otherwise it crashes with 0xbad00000
      std::shared_ptr<fgraphics_command_list> command_list = command_queue->get_command_list(ecommand_list_purpose::main, i);
      gpu_crash_handler.create_context_handle(i, command_list->com.Get());
    }
#endif
    command_queue->flush();

    LOG_INFO("Loading scene persistent state");
    scene_root = hscene::spawn();
    load_scene_state();
    
    LOG_INFO("Creating physics scene");
    using namespace reactphysics3d;
    physics_common = std::make_shared<PhysicsCommon>();
    DefaultLogger* logger = physics_common->createDefaultLogger();
    uint log_level = static_cast<uint>(static_cast<uint>(Logger::Level::Warning) | static_cast<uint>(Logger::Level::Error));
    logger->addStreamDestination(std::cout, log_level, DefaultLogger::Format::Text);
    physics_common->setLogger(logger);
    begin_physics();

    LOG_INFO("Initiating the window");
    window->init(WndProc, factory, L"Editor");
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
      float delta_time = stat_frame_time.get_last_time_ms() / 1000.0f;
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

  void fapplication::begin_physics()
  {
    using namespace reactphysics3d;

    PhysicsWorld::WorldSettings settings;
    settings.isSleepingEnabled = false;
    settings.gravity = Vector3(0, -9.81f, 0);
    physics_world = physics_common->createPhysicsWorld(settings);
    scene_root->create_scene_physics_state();
  }

  void fapplication::end_physics()
  {
    scene_root->destroy_scene_physics_state();
    physics_common->destroyPhysicsWorld(physics_world);
    physics_world = nullptr;
  }

  void fapplication::update_physics(float delta_time)
  {
    physics_world->update(delta_time);
    scene_root->update_scene_physics_state(delta_time);
  }

  void fapplication::update(float delta_time)
  {
    if(scene_root != nullptr && scene_root->renderer != nullptr)
    {
      if(wants_to_simulate_physics && !scene_root->is_simulating_physics)
      {
        scene_root->is_simulating_physics = true;
        scene_root->set_scene_physics_state();
      }
      else if(!wants_to_simulate_physics && scene_root->is_simulating_physics)
      {
        scene_root->is_simulating_physics = false;
        scene_root->reset_scene_physics_state();
      }
      if(scene_root->is_simulating_physics && delta_time != 0.0f)
      {
        update_physics(delta_time);
        
      }
      scene_root->camera.update(delta_time, scene_root->renderer->context.width, scene_root->renderer->context.height);
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
}