#include "stdafx.h"

#include "core/windows_minimal.h"

#include <d3d11_1.h>
#include <tchar.h>
#include <thread>
#include <DirectXColors.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "app/editor_app.h"
#include "renderers/cpu_renderer.h"
#include "renderer/dx11_lib.h"
#include "renderer/renderer_base.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 
namespace ray_tracer
{
  LRESULT editor_app::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
      return true;
    }
    return application::wnd_proc(hWnd, msg, wParam, lParam);
  }
  
  void editor_app::init()
  {
    application::init();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    {
      // overwrite imgui config file name
      std::string imgui_ini_filename = engine::io::get_imgui_file_path();
      char* buff = new char[imgui_ini_filename.size() + 1];
      strcpy(buff, imgui_ini_filename.c_str());  // returning char* is fucked up
      io.IniFilename = buff;
    }
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  
    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();
    ImGui::GetIO().KeyRepeatDelay = 0.1f;
    //ImGui::GetIO().KeyRepeatRate = 0.01f;
  
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    dx11& dx = dx11::instance();
    ImGui_ImplDX11_Init(dx.device, dx.device_context);
  
    // Load persistent state
    app_state.load_window_state();
    app_state.load_rendering_state();
    app_state.load_assets();
    app_state.load_scene_state();
    app_state.scene_root->load_resources();
    app_state.renderer = REG.spawn_from_class<renderer_base>(app_state.renderer_conf.type);
    ::SetWindowPos(hwnd, NULL, app_state.window_conf.x, app_state.window_conf.y, app_state.window_conf.w, app_state.window_conf.h, NULL);
  
    // Auto render on startup
    app_state.rw_model.rp_model.render_pressed = true;
  
    LOG_INFO("Loading done, starting the main loop");
  }

  void editor_app::run()
  {
    while (app_state.is_running)
    {
      pump_messages();
      if (!app_state.is_running) break;
  
      handle_input(app_state);
      manage_renderer();
      draw_scene();
      draw_ui();
      present();
      
      RECT rect;
      ::GetWindowRect(hwnd, &rect);
      app_state.window_conf.x = rect.left;
      app_state.window_conf.y = rect.top;
      app_state.window_conf.w = rect.right - rect.left;
      app_state.window_conf.h = rect.bottom - rect.top;
    }
  }
  
  void editor_app::cleanup()
  {
    app_state.save_window_state();
  
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    application::cleanup();
  }
  
  void editor_app::pump_messages()
  {
    // Poll and handle messages (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
      {
        app_state.is_running = false;
      }
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }
  }
  
  void editor_app::manage_renderer()
  {
    // Respawn the renderer if the type needs to be different
    if (app_state.renderer->get_class() != app_state.renderer_conf.new_type && app_state.renderer_conf.new_type != nullptr)
    {
      // Wait until existing one ends and kill it
      app_state.renderer->cancel();
      if(app_state.renderer->is_async())
      {
        while(app_state.renderer->is_working())
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
      app_state.renderer->destroy();
  
      // Add new one
      auto new_class = app_state.renderer_conf.new_type;
      auto new_renderer = REG.spawn_from_class<renderer_base>(new_class);
      app_state.renderer_conf.type = new_class;
      app_state.renderer = new_renderer;
    }
  }
  
  void editor_app::draw_ui()
  {
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
  
  #ifndef IMGUI_DISABLE_DEMO_WINDOWS
      // Debug UI only in debug mode
      if (0) { ImGui::ShowDemoWindow(); }
  #endif
  
    draw_raytracer_window(app_state.rw_model, app_state);
    draw_output_window(app_state.ow_model, app_state);
    draw_scene_editor_window(app_state.sew_model, app_state);
  
    ImGui::Render(); // Draw, prepare for render
  }
  
  void editor_app::draw_scene()
  {
    if (app_state.renderer != nullptr)
    {
      // CPU Rendering to a texture is happening in the separate thread.
      // Main loop updates UI and the texture output window while it is rendered.
      // UI framerate is locked to VSync frequency while scene rendering may take longer time.
      // For now, GPU rendering is blocking the main thread.
      const bool is_working_async = app_state.renderer->is_async() && app_state.renderer->is_working();
      if (!is_working_async && (app_state.rw_model.rp_model.render_pressed || app_state.ow_model.auto_render))
      {
        app_state.scene_root->load_resources();
        app_state.scene_root->pre_render();
        app_state.scene_root->build_boxes();
        app_state.scene_root->update_materials();
        app_state.scene_root->query_lights();
  
        update_default_spawn_position(app_state);
  
        app_state.output_width = app_state.renderer_conf.resolution_horizontal;
        app_state.output_height = app_state.renderer_conf.resolution_vertical;
  
        app_state.renderer->render_frame(app_state.scene_root, app_state.renderer_conf, app_state.camera_conf);
  
        app_state.rw_model.rp_model.render_pressed = false;
      }
      app_state.renderer->push_partial_update();
    }
  }
  
  void editor_app::present()
  {
    dx11& dx = dx11::instance();
    dx.device_context->OMSetRenderTargets(1, &dx.rtv, NULL);
    
    dx.device_context->ClearRenderTargetView(dx.rtv, DirectX::Colors::LightSlateGray);
        
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  
    dx.swap_chain->Present(1, 0); // Present with vsync
  }
}
