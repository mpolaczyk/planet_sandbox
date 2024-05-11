#include "stdafx.h"

#include "core/windows_minimal.h"

#include <d3d11_1.h>
#include <tchar.h>
#include <DirectXColors.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "app/editor_app.h"

#include "hittables/scene.h"
#include "renderer/dx11_lib.h"
#include "renderer/renderer_base.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 
namespace editor
{
  LRESULT feditor_app::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
      return true;
    }
    return fapplication::wnd_proc(hWnd, msg, wParam, lParam);
  }
  
  void feditor_app::init(const char* project_name)
  {
    ImGui_ImplWin32_EnableDpiAwareness();
    
    fapplication::init(project_name);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    {
      // overwrite imgui config file name
      std::string imgui_ini_filename = engine::fio::get_imgui_file_path();
      char* buff = new char[imgui_ini_filename.size() + 1];
      strcpy(buff, imgui_ini_filename.c_str());  // returning char* is fucked up
      io.IniFilename = buff;
    }
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  
    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
  
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    fdx11& dx = fdx11::instance();
    ImGui_ImplDX11_Init(dx.device.Get(), dx.device_context.Get());
  
    // Load persistent state
    app_state.load_window_state();
    
    app_state.load_assets();
    app_state.load_scene_state();
    app_state.scene_root->load_resources();
    app_state.renderer = REG.spawn_from_class<rrenderer_base>(app_state.scene_root->renderer_config.type);
    ::SetWindowPos(hwnd, NULL, app_state.window_conf.x, app_state.window_conf.y, app_state.window_conf.w, app_state.window_conf.h, NULL);
    
    LOG_INFO("Loading done, starting the main loop");
  }

  void feditor_app::run()
  {
    while (app_state.is_running)
    {
      pump_messages();
      if (!app_state.is_running) break;

      const ImGuiIO& io = ImGui::GetIO();
      app_state.app_delta_time_ms = io.DeltaTime * 1000.0f;
      app_state.render_delta_time_ms = static_cast<float>(app_state.renderer->get_render_time_ms());
      
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
  
  void feditor_app::cleanup()
  {
    app_state.save_window_state();
  
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    fapplication::cleanup();
  }
  
  void feditor_app::pump_messages()
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
  
  void feditor_app::manage_renderer()
  {
    // Respawn the renderer if the type needs to be different
    frenderer_config& renderer_config = app_state.scene_root->renderer_config;
    if (app_state.renderer->get_class() != renderer_config.new_type && renderer_config.new_type != nullptr)
    {
      app_state.renderer->destroy();
  
      // Add new one
      auto new_class = renderer_config.new_type;
      auto new_renderer = REG.spawn_from_class<rrenderer_base>(new_class);
      renderer_config.type = new_class;
      app_state.renderer = new_renderer;
    }
  }
  
  void feditor_app::draw_ui()
  {
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
  
  #ifndef IMGUI_DISABLE_DEMO_WINDOWS
      // Debug UI only in debug mode
      if (0) { ImGui::ShowDemoWindow(); }
  #endif
  
    draw_editor_window(app_state.editor_window_model, app_state);
    draw_output_window(app_state.output_window_model, app_state);
    draw_scene_window(app_state.scene_window_model, app_state);
  
    ImGui::Render(); // Draw, prepare for render
  }
  
  void feditor_app::draw_scene()
  {
    if (app_state.renderer != nullptr)
    {
        app_state.scene_root->load_resources();
  
        update_default_spawn_position(app_state);

        const frenderer_config& renderer_config = app_state.scene_root->renderer_config;
      
        app_state.output_width = renderer_config.resolution_horizontal;
        app_state.output_height = renderer_config.resolution_vertical;

        app_state.scene_root->camera_config.update(app_state.app_delta_time_ms / 1000.0f);
        
        app_state.renderer->render_frame(app_state.scene_root, app_state.selected_object);
    }
  }
  
  void feditor_app::present()
  {
    fdx11& dx = fdx11::instance();
    dx.device_context->OMSetRenderTargets(1, dx.rtv.GetAddressOf(), nullptr);
    
    dx.device_context->ClearRenderTargetView(dx.rtv.Get(), DirectX::Colors::LightSlateGray);
        
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  
    dx.swap_chain->Present(1, 0); // Present with vsync
  }
}
