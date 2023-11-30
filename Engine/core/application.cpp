#include "core/application.h"

#include "object/object_registry.h"
#include "engine/log.h"

namespace engine
{
  void application::run()
  {
    logger::init();
    get_object_registry()->create_class_objects();
  }
}