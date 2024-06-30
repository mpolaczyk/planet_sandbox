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
    static fapplication* instance;

    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void init(const char* project_name);
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void cleanup();

    void main_loop();
    
    bool is_running = true;
    std::shared_ptr<fwindow> window;
    
  protected:
    ComPtr<ID3D12Device> device;
    std::shared_ptr<fcommand_queue> command_queue;
  };

  fapplication* create_application();
}
