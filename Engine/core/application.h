#pragma once

#include <memory>
#include <wrl/client.h>

#include "core/core.h"

struct ID3D12Device;

using Microsoft::WRL::ComPtr;

namespace engine
{  
  struct fcommand_queue;
  class fwindow;

  class ENGINE_API fapplication
  {
  public:
    static fapplication* app_weak_ptr;

    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void init(const char* project_name);
    virtual void run() = 0;
    virtual fwindow spawn_window() = 0;
    virtual void cleanup();

  protected:
    ComPtr<ID3D12Device> device;
    std::shared_ptr<fcommand_queue> command_queue;
    std::shared_ptr<fwindow> window;
  };

  fapplication* create_application();
}
