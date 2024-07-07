#pragma once

#include <memory>
#include <wrl/client.h>

#include "core/core.h"
#include "math/camera.h"

struct ID3D12Device2;

using Microsoft::WRL::ComPtr;

namespace engine
{
  class hscene;
  struct fcommand_queue;
  class fwindow;

  class ENGINE_API fapplication
  {
  public:
    static fapplication* instance;

    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void init(const char* project_name);
    virtual void update();
    virtual void cleanup();

    void main_loop();

    float app_delta_time_ms = 0.0f;
    float render_delta_time_ms = 0.0f;
    
    hscene* scene_root = nullptr;
    fcamera camera;
    
    bool is_running = true;
    std::shared_ptr<fwindow> window;
    
    ComPtr<ID3D12Device2> device;
    std::shared_ptr<fcommand_queue> command_queue;
  };

  fapplication* create_application();
}
