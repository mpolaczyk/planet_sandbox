#include "stdafx.h"

#include "core/windows_minimal.h"

#include <tchar.h>

#include "imgui.h"
#include "imgui_impl_win32.h"

#include "app/editor_app.h"

#include "app/editor_window.h"
#include "hittables/scene.h"
#include "renderer/command_queue.h"

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

    fapplication::init(project_name);

    load_assets();
  }

  feditor_window* feditor_app::get_editor_window() const
  {
    return static_cast<feditor_window*>(window.get());
  }
}