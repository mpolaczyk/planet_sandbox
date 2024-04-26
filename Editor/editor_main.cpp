#include "stdafx.h"

#include "app/editor_app.h"

namespace engine
{
  fapplication* create_application()
  {
    // Engine will use editor_app instance, see entry_point.h
    return new editor::feditor_app();
  }
}