#pragma once

#include "app/app.h"
#include "renderer/command_queue.h"

namespace editor
{
  class feditor_app final : public fapplication
  {
  public:
    // Parent class: application interface
    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    virtual void init(const char* project_name) override;
    virtual void run() override;
    virtual fwindow spawn_window() override;
    virtual void cleanup() override;

  private:
    void pump_messages();
    void draw_ui();
    void draw_scene();
    void render();
    
    fapp_instance app_state;
  };
}