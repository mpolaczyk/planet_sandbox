#pragma once

#include <string>

#include "core/core.h"
#include "object/object_defines.h"

/* Adding new object classes:
 * 1. Add new class, derive from object.
 * 2. Add OBJECT_DECLARE in the header file.
 * 3. Add OBJECT_DEFINE in the cpp file.
 * 4. Add OBJECT_DEFINE_SPAWN or OBJECT_DEFINE_NOSPAWN in the cpp file. Depending if your class is meant to be spawned.
 * 5. Optional: Add OBJECT_DECLARE_VISITOR and OBJECT_DEFINE_VISITOR in header and cpp files depending if you want
 *    to use the visitor pattern based operations on the type.
 * 6. Register class in object_registry::create_class_objects() using CLASS_OBJECT_REGISTER.
 * 7. Instantiate template methods using OBJECT_REGISTRY_EXPLICIT_INSTANTIATE in the same file.
 * 8. Optional: Extend the serialize_object and deserialize_object visitors if you wish to serialize instances.
 */

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
    OBJECT_DECLARE_VISITOR_BASE
    
    object() = default;
    object(const object&) = default;
    object& operator=(const object&) = default;
	  object(object&&) = delete;
	  object& operator=(object&&) = delete;

    int get_runtime_id() const;

    void destroy();

  private:
    // Runtime id can be set only once by the registry. Can't change at runtime.
    void set_runtime_id(int id);
    int runtime_id = -1;
  };

  typedef object*(*spawn_instance_func_type)();
  
  // Base class for all class types. Each registered object has object class instance in the registry.
  class ENGINE_API class_object : public object
  {
    friend object_registry;
    
  public:
    OBJECT_DECLARE(class_object, object)

    std::string get_class_name() const { return class_name; }
    std::string get_parent_class_name() const { return parent_class_name; }
    
  private:
    spawn_instance_func_type spawn_instance_func;
    std::string class_name;
    std::string parent_class_name;
  };
}