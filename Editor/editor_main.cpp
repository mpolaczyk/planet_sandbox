#include "stdafx.h"

#include "app/editor_app.h"
#include "app/editor_window.h"

namespace engine
{
  // Engine will use editor_app and feditor_window, see entry_point.h
  
  fapplication* create_application()
  {
    return new editor::feditor_app();
  }

  fwindow* create_window()
  {
    return new editor::feditor_window();
  }
}