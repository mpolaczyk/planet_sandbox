#pragma once

#include "core/core.h"
#include "object/object.h"

// Put this in the h file, implement persistent load/save together with assets.
#define OBJECT_DECLARE_LOAD(CLASS_NAME) static bool load(CLASS_NAME* instance, const std::string& name);
#define OBJECT_DECLARE_SAVE(CLASS_NAME) static void save(CLASS_NAME* instance);

namespace engine
{
  // Base class for all assets, don't instantiate
  class ENGINE_API aasset_base : public oobject
  {
  public:
    OBJECT_DECLARE(aasset_base, oobject)
    OBJECT_DECLARE_LOAD(aasset_base)
    
    // JSON file name
    std::string file_name;
  };
}
