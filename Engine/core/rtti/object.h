#pragma once

#include <string>

#include "core/core.h"
#include "core/rtti/object_defines.h"

/* Adding new oobject classes:
 * 1. Add new class, derive from object.
 * 2. Add OBJECT_DECLARE in the header file.
 * 3. Add OBJECT_DEFINE in the cpp file.
 * 4. Add OBJECT_DEFINE_SPAWN or OBJECT_DEFINE_NOSPAWN in the cpp file. Depending if your class is meant to be spawned.
 * 5. Optional: Add OBJECT_DECLARE_VISITOR and OBJECT_DEFINE_VISITOR in header and cpp files depending if you want
 *    to use the visitor pattern based operations on the type.
 * 6. Register class in object_registry::create_class_objects() using CLASS_OBJECT_REGISTER.
 * 7. Instantiate template methods using OBJECT_REGISTRY_EXPLICIT_INSTANTIATE in the same file.
 * 8. Optional: Extend the vserialize_object and vdeserialize_object visitors if you wish to serialize instances.
 */

namespace engine
{
  class fobject_registry;
  class oclass_object;

  // Managed object class
  // Base class for all objects, use like abstract
  class ENGINE_API oobject
  {
  private:
    friend fobject_registry;

  public:
    OBJECT_DECLARE(oobject, oobject)
    OBJECT_DECLARE_VISITOR_BASE

    CTOR_DEFAULT(oobject)
    CTOR_COPY_DEFAULT(oobject)
    CTOR_MOVE_DELETE(oobject)
    VDTOR_DEFAULT(oobject)
    
    int get_runtime_id() const;

    virtual uint32_t get_hash() const;
    virtual void destroy();

  private:
    // Runtime id can be set only once by the registry. Can't change at runtime.
    void set_runtime_id(int id);
    int runtime_id = -1; // TODO uint32_t
  };

  typedef oobject*(*spawn_instance_func_type)();

  // Base class for all class types. Each registered object has object class instance in the registry.
  class ENGINE_API oclass_object : public oobject
  {
    friend fobject_registry;

  public:
    OBJECT_DECLARE(oclass_object, oobject)

    std::string get_class_name() const { return class_name; }
    std::string get_parent_class_name() const { return parent_class_name; }

  private:
    spawn_instance_func_type spawn_instance_func{};
    std::string class_name;
    std::string parent_class_name;
  };
}
