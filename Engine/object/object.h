#pragma once

#include "core/core.h"

#include "object/object_types.h"
#include "object/object_defines.h"

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
    virtual std::string get_name() const;

    int get_runtime_id() const;

    void destroy();

  private:
    // Runtime id Can be set only once by the registry. Can't change at runtime.
    void set_runtime_id(int id);
    int runtime_id = -1;
  };
}