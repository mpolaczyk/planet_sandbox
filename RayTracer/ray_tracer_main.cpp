#include "stdafx.h"

#include "app/editor_app.h"

namespace engine
{
  application* create_application()
  {
    // Engine will use editor_app instance, see entry_point.h
    return new ray_tracer::editor_app();
  }
}