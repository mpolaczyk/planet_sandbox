#pragma once

#include "core/application.h"
#include "math/camera.h"

namespace engine
{
  class hscene;
  class fwindow;
}

namespace editor
{
  class feditor_window;
  
  class feditor_app final : public fapplication
  {
  public:
    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    virtual void init(const char* project_name) override;
    virtual void update() override;
    virtual void draw() override;
    virtual void cleanup() override;

    void load_scene_state() const;
    void save_scene_state() const;
    void load_window_state() const;
    void save_window_state() const;

    static void load_assets();
    static void save_materials();
    
    float app_delta_time_ms = 0.0f;
    float render_delta_time_ms = 0.0f;

    hscene* scene_root = nullptr;
    fcamera camera;
    
  private:
    feditor_window* get_editor_window() const;
    
    void render();
  };
}