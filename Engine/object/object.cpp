
#include <assert.h>
#include <sstream>  

#include "object.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(class_object, object)
  OBJECT_DEFINE_SPAWN(class_object)

  OBJECT_DEFINE(object, object)
  OBJECT_DEFINE_NOSPAWN(object)

  std::string object::get_display_name() const
  {
    std::ostringstream oss;
    //oss << "[" << runtime_id << "] " << object_type_names[static_cast<int>(get_class())] << ": " << get_name(); // FIX
    oss << "[" << runtime_id << "] " << get_class()->get_name() << ":" << get_name();
    return oss.str();
  }

  void object::set_runtime_id(int id)
  {
    if (runtime_id == -1)
    {
      runtime_id = id;
    }
  }

  int object::get_runtime_id() const
  {
    return runtime_id;
  }

  void object::destroy()
  {
    get_object_registry()->destroy(runtime_id);
  }

  std::string object::get_name() const
  {
    return get_object_registry()->get_name(runtime_id);
  }
}