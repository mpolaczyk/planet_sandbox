#pragma once

#include <memory>

#include "core/core.h"
#include "core/com_pointer.h"
#include "engine/math/camera.h"
#include "engine/profile/benchmark.h"

#if USE_NSIGHT_AFTERMATH
#include "gpu_crash_handler.h"
#endif

namespace reactphysics3d
{
  class PhysicsCommon;
  class PhysicsWorld;
}

namespace engine
{
  class rrenderer_base;
  class hscene;
  class fwindow;
  struct fcommand_queue;
  struct fdevice;
  struct fphysics;

  class ENGINE_API fapplication
  {
  private:
    static fapplication* instance;
    static uint64_t frame_counter;
    
  public:

    CTOR_DEFAULT(fapplication)
    CTOR_MOVE_COPY_DELETE(fapplication)
    virtual ~fapplication();

    static ftimer_instance stat_frame_time;
    static ftimer_instance stat_update_time;
    static ftimer_instance stat_draw_time;
    static ftimer_instance stat_render_time;
    
    static void set_instance(fapplication* value) { instance = value; }
    static fapplication* get_instance() { return instance; };
    
    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void init(const char* project_name);
    virtual void update(float delta_time);
    virtual void draw();
    virtual void render();
    void load_scene_state() const;
    void save_scene_state() const;
    void try_hot_swap_shaders();
    void log_heap_descriptors() const;

    void set_window(fwindow* in_window);
    void main_loop();

    std::shared_ptr<fphysics> physics;
    hscene* scene_root = nullptr; // managed object does not work well with shared_ptr, because they need to be destroyed through object registry
    
    bool is_running = true;
    std::shared_ptr<fwindow> window;
    std::shared_ptr<fdevice> device;
    std::shared_ptr<fcommand_queue> command_queue;

#if USE_NSIGHT_AFTERMATH
    fgpu_crash_tracker gpu_crash_handler;
#endif
  };

  fapplication* create_application();
}
