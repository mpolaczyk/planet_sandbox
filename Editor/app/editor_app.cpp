#include "stdafx.h"

#include "core/windows_minimal.h"

#include <tchar.h>
#include "d3dx12/d3dx12_core.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

#include "app/editor_app.h"

#include "app/editor_window.h"
#include "hittables/scene.h"
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

  feditor_window* feditor_app::get_editor_window() const
  {
    return static_cast<feditor_window*>(window.get());
  }
  
  void feditor_app::init(const char* project_name)
  {
    ImGui_ImplWin32_EnableDpiAwareness();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    fapplication::init(project_name);

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    {
      // overwrite imgui config file name
      std::string imgui_ini_filename = engine::fio::get_imgui_file_path();
      char* buff = new char[imgui_ini_filename.size() + 1];
      strcpy(buff, imgui_ini_filename.c_str()); // returning char* is fucked up
      io.IniFilename = buff;
    }
    
    // Load persistent state
    load_assets();
    scene_root = hscene::spawn();
    load_scene_state();
    scene_root->load_resources();
  }

  void feditor_app::update()
  {
    const ImGuiIO& io = ImGui::GetIO();
    app_delta_time_ms = io.DeltaTime * 1000.0f;
    render_delta_time_ms = static_cast<float>(scene_root->renderer->get_render_time_ms());
  }

  void feditor_app::draw()
  {
    if(scene_root != nullptr)
    {
      rrenderer_base* renderer = scene_root->renderer;
      if(renderer != nullptr)
      {
        scene_root->load_resources();

        get_editor_window()->update_default_spawn_position();

        scene_root->camera_config.update(app_delta_time_ms / 1000.0f, renderer->output_width, renderer->output_height);

        renderer->render_frame(scene_root, get_editor_window()->selected_object);
      }
    }
  }
  
  void feditor_app::cleanup()
  {
    if (scene_root)
    {
      scene_root->destroy();
    }
    fapplication::cleanup();
  }
}