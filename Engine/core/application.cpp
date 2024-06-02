#include <d3d11_1.h>

#include <tchar.h>

#include "core/application.h"


#include "engine/io.h"
#include "object/object_registry.h"
#include "engine/log.h"
#include "math/random.h"
#include "renderer/dx11_lib.h"
#include "resources/assimp_logger.h"

namespace engine
{
  fapplication* fapplication::app_weak_ptr = nullptr;

  LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return fapplication::app_weak_ptr->wnd_proc(hWnd, msg, wParam, lParam);
  }

  LRESULT fapplication::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    fdx11& dx = fdx11::instance();
    switch(msg)
    {
    case WM_SIZE:
      if(dx.device != NULL && wParam != SIZE_MINIMIZED)
      {
        dx.cleanup_render_target();
        dx.swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
        dx.create_render_target();
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
    fassimp_logger::initialize();
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

    // Window
    wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Editor"), NULL};
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, _T("Editor"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    fdx11& dx = fdx11::instance();
    dx.create_device();
    dx.create_debug_layer();
    dx.create_swap_chain(hwnd);
    dx.create_render_target();

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
  }

  void fapplication::cleanup()
  {
    fdx11& dx = fdx11::instance();
    dx.cleanup_device();
    dx.cleanup_render_target();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  }
}
