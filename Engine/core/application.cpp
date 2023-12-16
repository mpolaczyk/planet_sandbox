#include "core/application.h"

#include "object/object_registry.h"
#include "engine/log.h"

namespace engine
{
  void application::run()
  {
    logger::init();
    REG.create_class_objects();
  }
}