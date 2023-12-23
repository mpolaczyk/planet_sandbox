#include "stdafx.h"

#include "core/windows_minimal.h"

#include <thread>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11_1.h>
#include <tchar.h>

#include "app/app.h"
#include "renderer/dx11_lib.h"
#include "renderer/async_renderer_base.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  dx11& dx = dx11::instance();
  switch (msg)
  {
  case WM_SIZE:
    if (dx.device != NULL && wParam != SIZE_MINIMIZED)
    {
      dx.cleanup_render_target();
      dx.swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
      dx.create_render_target();
    }
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

int app_main()
{
#if USE_FPEXCEPT
  fpe_enabled_scope fpe;
#endif

  LOG_INFO("Working dir: {0}", engine::io::get_working_dir());
  LOG_INFO("Workspace dir: {0}", engine::io::get_workspace_dir());
  if (!engine::io::validate_workspace_dir())
  {
      LOG_CRITICAL("Invalid workspace directory!");
  }

  // Create application window
  //ImGui_ImplWin32_EnableDpiAwareness();
  WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RayTracer"), NULL };
  ::RegisterClassEx(&wc);
  HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("RayTracer"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);

  // Initialize Direct3D
  dx11& dx = dx11::instance();
  if (!dx.create_device())
  {
    dx.cleanup_device();
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 1;
  }
  dx.create_debug_layer();
  dx.create_swap_chain(hwnd);
  dx.create_render_target();
  
  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

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
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();
  ImGui::GetIO().KeyRepeatDelay = 0.1f;
  //ImGui::GetIO().KeyRepeatRate = 0.01f;

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX11_Init(dx.device, dx.device_context);

  // Raytracer init
  random_cache::init();

  // Load persistent state
  app_instance app_state;
  app_state.load_window_state();
  app_state.load_rendering_state();
  app_state.load_assets();
  app_state.load_scene_state();
  app_state.scene_root->load_resources();
  app_state.renderer = REG.spawn_from_class<async_renderer_base>(app_state.renderer_conf.type);
  ::SetWindowPos(hwnd, NULL, app_state.window_conf.x, app_state.window_conf.y, app_state.window_conf.w, app_state.window_conf.h, NULL);

  // Auto render on startup
  app_state.rw_model.rp_model.render_pressed = true;

  LOG_INFO("Loading done, starting the main loop");

  // Main loop
  while (app_state.is_running)
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
    if (!app_state.is_running) break;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    handle_input(app_state);

    if (app_state.renderer->is_renderer_type_different(app_state.renderer_conf))
    {
      app_state.renderer->destroy();
      app_state.renderer = REG.spawn_from_class<async_renderer_base>(app_state.renderer_conf.type);
    }
    
    // Draw UI
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
      // Debug UI only in debug mode
      if (0) { ImGui::ShowDemoWindow(); }
#endif
    draw_raytracer_window(app_state.rw_model, app_state);
    draw_output_window(app_state.ow_model, app_state);
    draw_scene_editor_window(app_state.sew_model, app_state);

    // Scene rendering
    if (app_state.renderer != nullptr)
    {
      // Rendering to a texture is happening in the separate thread.
      // Main loop updates UI and the texture output window while it is rendered.
      // UI framerate is locked to VSync frequency while scene rendering may take longer time.
      const bool is_working = app_state.renderer->is_working();
      if (!is_working && (app_state.rw_model.rp_model.render_pressed || app_state.ow_model.auto_render))
      {
        app_state.scene_root->load_resources();
        app_state.scene_root->pre_render();
        app_state.scene_root->build_boxes();
        app_state.scene_root->update_materials();
        app_state.scene_root->query_lights();

        update_default_spawn_position(app_state);

        app_state.output_width = app_state.renderer_conf.resolution_horizontal;
        app_state.output_height = app_state.renderer_conf.resolution_vertical;

        app_state.renderer->render_single_async(app_state.scene_root, app_state.renderer_conf, app_state.camera_conf);

        bool ret = dx.load_texture_from_buffer(app_state.renderer->get_img_rgb(), app_state.output_width, app_state.output_height, &app_state.output_srv, &app_state.output_texture);
        IM_ASSERT(ret);

        app_state.rw_model.rp_model.render_pressed = false;
      }

      // Updating the texture output window so that the scene render is visible in UI
      if (app_state.output_texture)
      {
        dx.update_texture_buffer(app_state.renderer->get_img_rgb(), app_state.output_width, app_state.output_height, app_state.output_texture);
      }
    }

    // UI rendering
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static float clear_color_with_alpha[4] =
    {
      clear_color.x * clear_color.w,
      clear_color.y * clear_color.w,
      clear_color.z * clear_color.w,
      clear_color.w
    };
    ImGui::Render(); // Draw, prepare for render
    dx.device_context->OMSetRenderTargets(1, &dx.frame_buffer_view, NULL);
    dx.device_context->ClearRenderTargetView(dx.frame_buffer_view, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (true)
    {
      dx.swap_chain->Present(1, 0); // Present with vsync
    }
    else
    {
      // Hardcoded frame limiter
      std::this_thread::sleep_for(std::chrono::milliseconds(40));
      dx.swap_chain->Present(0, 0); // Present without vsync
    }

    RECT rect;
    ::GetWindowRect(hwnd, &rect);
    app_state.window_conf.x = rect.left;
    app_state.window_conf.y = rect.top;
    app_state.window_conf.w = rect.right - rect.left;
    app_state.window_conf.h = rect.bottom - rect.top;
  }

  app_state.save_window_state();

  // Cleanup
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  dx.cleanup_device();
  dx.cleanup_render_target();
  ::DestroyWindow(hwnd);
  ::UnregisterClass(wc.lpszClassName, wc.hInstance);

  return 0;
}


class editor_app : public engine::application
{
public:

  virtual void run() override
  {
    application::run();
    app_main();
  }
};

engine::application* engine::create_appliation()
{
  return new editor_app();
}

