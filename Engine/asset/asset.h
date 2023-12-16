#pragma once

#include "core/core.h"
#include "object/object.h"

// Put this in the h file, implement persistent load/save together with assets.
#define OBJECT_DECLARE_LOAD(CLASS_NAME) static bool load(CLASS_NAME* instance, const std::string& name);
#define OBJECT_DECLARE_SAVE(CLASS_NAME) static void save(CLASS_NAME* instance);

namespace engine
{
  // Base class for all assets, don't instantiate
  class ENGINE_API asset_base : public object
  {
  public:
    OBJECT_DECLARE(asset_base, object)
    OBJECT_DECLARE_LOAD(asset_base)
    
    // File system name
    std::string file_name;
  };
}
