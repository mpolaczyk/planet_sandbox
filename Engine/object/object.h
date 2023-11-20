#pragma once

#include "core/core.h"

#include "object/object_defines.h"
#include "object/object_types.h"

#include <string>

namespace engine
{
  class object_registry;

  // Managed object class
  // Base class for all objects, use like abstract
  class ENGINE_API object
  {
    friend object_registry;

  public:
    OBJECT_DECLARE(object, object)

    virtual std::string get_display_name() const;

    void set_runtime_id(int id);
    int get_runtime_id() const;

    std::string get_name() const;

  private:

    // Can be set only once by the registry, index in the vector
    // Can't change at runtime, can't be cloned
    int runtime_id = -1;
  };
}