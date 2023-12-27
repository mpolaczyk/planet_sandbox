#pragma once

#include "app/app.h"

namespace ray_tracer
{
  class editor_app final : public application
  {
  public:
    // Parent class: application interface
    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    virtual void init() override;
    virtual void run() override;
    virtual void cleanup() override;
    
  private:
    void pump_messages();
    void manage_renderer();
    void draw_ui();
    void draw_scene();
    void present();

    app_instance app_state;
  };
}
