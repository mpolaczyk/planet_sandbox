#include "stdafx.h"

#include "core/windows_minimal.h"

#include <tchar.h>
#include <DirectXColors.h>
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_barriers.h"
#include "d3dx12/d3dx12_core.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "app/editor_app.h"

#include <dxgi.h>
#include <dxgi1_4.h>

#include "core/window.h"
#include "hittables/scene.h"
#include "renderer/dx12_lib.h"
#include "renderer/renderer_base.h"
#include "renderers/gpu_forward_sync.h"

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
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    {
      // overwrite imgui config file name
      std::string imgui_ini_filename = engine::fio::get_imgui_file_path();
      char* buff = new char[imgui_ini_filename.size() + 1];
      strcpy(buff, imgui_ini_filename.c_str()); // returning char* is fucked up
      io.IniFilename = buff;
    }
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(window->hwnd);
    ImGui_ImplDX12_Init(device.Get(), window->back_buffer_count, DXGI_FORMAT_R8G8B8A8_UNORM, window->srv_descriptor_heap.Get(),
                        window->srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), window->srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());

    // Load persistent state
    app_state.load_window_state();

    app_state.load_assets();
    app_state.load_scene_state();
    app_state.scene_root->load_resources();
    
    ::SetWindowPos(window->hwnd, NULL, app_state.window_conf.x, app_state.window_conf.y, app_state.window_conf.w, app_state.window_conf.h, NULL);

    LOG_INFO("Loading done, starting the main loop");
  }

  void feditor_app::run()
  {
    while(app_state.is_running)
    {
      pump_messages();
      if(!app_state.is_running) break;

      const ImGuiIO& io = ImGui::GetIO();
      app_state.app_delta_time_ms = io.DeltaTime * 1000.0f;
      app_state.render_delta_time_ms = static_cast<float>(app_state.scene_root->renderer->get_render_time_ms());

      handle_input(app_state);
      draw_scene();
      draw_ui();
      render();

      RECT rect;
      ::GetWindowRect(window->hwnd, &rect);
      app_state.window_conf.x = rect.left;
      app_state.window_conf.y = rect.top;
      app_state.window_conf.w = rect.right - rect.left;
      app_state.window_conf.h = rect.bottom - rect.top;
    }
  }

  fwindow feditor_app::spawn_window()
  {
    return fwindow(L"Editor");    // TODO use editor window class
  }

  void feditor_app::cleanup()
  {
    app_state.save_window_state();

    ImGui_ImplDX12_Shutdown();
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
    while(::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
      if(msg.message == WM_QUIT)
      {
        app_state.is_running = false;
      }
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }
  }

  void feditor_app::draw_ui()
  {
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
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
    hscene* scene = app_state.scene_root;
    if(scene != nullptr)
    {
      rrenderer_base* renderer = scene->renderer;
      if(renderer != nullptr)
      {
        scene->load_resources();

        update_default_spawn_position(app_state);

        scene->camera_config.update(app_state.app_delta_time_ms / 1000.0f, renderer->output_width, renderer->output_height);

        renderer->render_frame(scene, app_state.selected_object);
      }
    }
  }

  void feditor_app::render()
  {
    fdx12& dx = fdx12::instance();
    
    auto command_list = command_queue->get_command_list(window->back_buffer_index);
    
    command_list->SetGraphicsRootSignature(window->root_signature.Get());

    CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(app_state.window_conf.w), static_cast<float>(app_state.window_conf.h));
    command_list->RSSetViewports(1, &viewport);
    
    CD3DX12_RECT scissor_rect = CD3DX12_RECT(0, 0, static_cast<LONG>(app_state.window_conf.w), static_cast<LONG>(app_state.window_conf.h));
    command_list->RSSetScissorRects(1, &scissor_rect);

    CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(window->rtv[window->back_buffer_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    command_list->ResourceBarrier(1, &resource_barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(window->rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), window->back_buffer_index, window->rtv_descriptor_size);
    command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
    command_list->ClearRenderTargetView(rtv_handle, DirectX::Colors::LightSlateGray, 0, nullptr);

    command_list->SetDescriptorHeaps(1, window->srv_descriptor_heap.GetAddressOf());
    
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());

    resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(window->rtv[window->back_buffer_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    command_list->ResourceBarrier(1, &resource_barrier);
    
    uint64_t fence_value = command_queue->execute_command_list(window->back_buffer_index);
    window->present();
    command_queue->wait_for_fence_value(fence_value);
  }
}