#pragma once

#include "core/windows_minimal.h"
#include "core/core.h"

namespace engine
{  
  class ENGINE_API fapplication
  {
  public:
    static fapplication* app_weak_ptr;
    
    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void init(const char* project_name);
    virtual void run() = 0; // Implement main loop here
    virtual void cleanup();
    
  protected:
    HWND hwnd;
    WNDCLASSEX wc;
  };
  
  fapplication* create_application();
}


