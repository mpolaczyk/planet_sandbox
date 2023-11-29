#include "core/application.h"
#include "object/object_registry.h"

namespace engine
{
  application::application()
  {
    get_object_registry()->create_class_objects();
  }

  application::~application()
  {

  }

  void application::run()
  {
    while (true);
  }
}