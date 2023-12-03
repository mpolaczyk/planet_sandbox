#pragma once

#include "core/core.h"
#include "core/concepts.h"

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
    // Runtime id can be set only once by the registry. Can't change at runtime.
    void set_runtime_id(int id);
    int runtime_id = -1;
  };

  typedef object*(*spawn_instance_func_type)(const std::string&);

  // Base class for all class types. Each registered object has object class instance in the registry.
  class ENGINE_API class_object : public object
  {
  public:
    OBJECT_DECLARE(class_object, object)

    template<derives_from<object> T>
    T* spawn_instance(const std::string& name) const;

    // FIX make privare, add getters, friend what is needed
    spawn_instance_func_type spawn_instance_func;
    std::string class_name;
    std::string parent_class_name;
  };
}