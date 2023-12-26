

#include <d3d11_1.h>

#include <tchar.h>

#include "core/application.h"


#include "engine/io.h"
#include "object/object_registry.h"
#include "engine/log.h"
#include "math/random.h"
#include "renderer/dx11_lib.h"

namespace engine
{
  application* application::app_weak_ptr = nullptr;
  
  LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return application::app_weak_ptr->wnd_proc(hWnd, msg, wParam, lParam);
  }

  LRESULT application::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
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
  
  void application::init()
  {
    logger::init();
    random_cache::init();
    
    REG.create_class_objects();

    LOG_INFO("Working dir: {0}", io::get_working_dir());
    LOG_INFO("Workspace dir: {0}", io::get_workspace_dir());
    if (!io::validate_workspace_dir())
    {
      LOG_CRITICAL("Invalid workspace directory!");
    }

    // Window
    wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RayTracer"), NULL };
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, _T("RayTracer"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    dx11& dx = dx11::instance();
    if (!dx.create_device())
    {
      dx.cleanup_device();
      ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    }
    dx.create_debug_layer();
    dx.create_swap_chain(hwnd);
    dx.create_render_target();
  
    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
  }

  void application::cleanup()
  {
    dx11& dx = dx11::instance();
    dx.cleanup_device();
    dx.cleanup_render_target();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  }

}
