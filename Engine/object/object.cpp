
#include <assert.h>
#include <sstream>  

#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE_BASE(object)
  OBJECT_DEFINE_NOSAVE(object)
  OBJECT_DEFINE_NOLOAD(object)

  std::string object::get_display_name() const
  {
    std::ostringstream oss;
    oss << "[" << runtime_id << "] " << object_type_names[static_cast<int>(get_type())] << ": " << get_name();
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

  std::string object::get_name() const
  {
    return get_object_registry()->get_name(runtime_id);
  }

  object_type object::get_type() const
  {
    return get_object_registry()->get_type(runtime_id);
  }
}