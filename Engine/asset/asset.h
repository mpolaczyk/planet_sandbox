#pragma once

#include "core/core.h"
#include "object/object.h"

// Put this in the h file, implement persistent load/save together with assets.
#define OBJECT_DECLARE_LOAD(CLASS_NAME) static bool load(CLASS_NAME* instance, const std::string& name);
#define OBJECT_DECLARE_SAVE(CLASS_NAME) static void save(CLASS_NAME* instance);
// TODO convert to member, remove the macro

namespace engine
{
  // Base class for all assets, don't instantiate
  class ENGINE_API aasset_base : public oobject
  {
  public:
    OBJECT_DECLARE(aasset_base, oobject)

    virtual const char* get_extension() = 0;
    OBJECT_DECLARE_LOAD(aasset_base)
    // OBJECT_DECLARE_SAVE(aasset_base) // TODO this is missing, so far file_name is handled in each child class. This is inconsistent, fix it.
    
    // JSON file name
    std::string file_name;
  };
}
