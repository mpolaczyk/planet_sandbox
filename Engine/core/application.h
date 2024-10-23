#pragma once

#include <memory>
#include <wrl/client.h>

#include "core/core.h"
#include "math/camera.h"
#include "profile/benchmark.h"

#if USE_NSIGHT_AFTERMATH
#include "gpu_crash_handler.h"
#endif

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
    static ftimer_instance stat_frame_time;
    static ftimer_instance stat_update_time;
    static ftimer_instance stat_draw_time;
    static ftimer_instance stat_render_time;
    static uint64_t frame_counter;

    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void init(const char* project_name);
    virtual void update();
    virtual void draw(int back_buffer_index);
    virtual void render(int back_buffer_index);
    virtual void cleanup();

    void main_loop();
    
    hscene* scene_root = nullptr;
    fcamera camera;
    
    bool is_running = true;
    std::shared_ptr<fwindow> window;
    
    ComPtr<ID3D12Device2> device;
    std::shared_ptr<fcommand_queue> command_queue;

#if USE_NSIGHT_AFTERMATH
    fgpu_crash_tracker gpu_crash_handler;
#endif
  };

  fapplication* create_application();
}
