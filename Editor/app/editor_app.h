#pragma once

#include "core/application.h"

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
    CTOR_DEFAULT(feditor_app)
    
    virtual LRESULT wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    virtual void init(const char* project_name) override;
    
    void load_window_state() const;
    void save_window_state() const;
    
    static void load_assets();
    static void save_materials();
    
  private:
    feditor_window* get_editor_window() const;
  };
}