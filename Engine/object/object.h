#pragma once

#include "core/core.h"

#include "object/object_types.h"
#include "object/object_defines.h"

#include <string>

namespace engine
{
  class object_registry;
  class class_object;

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

  // Base class for all class types. Each registered object has object class instance in the registry.
  class ENGINE_API class_object : public object
  {
  public:
    OBJECT_DECLARE(class_object, object)

      // FIX make privare, add getters, friend what is needed
    std::string class_name;
    std::string parent_class_name;
  };
}