
#include <assert.h>
#include <sstream>  

#include "asset/asset_registry.h"

namespace engine
{
  object_type object::get_static_type()
  {
    assert(false); // Not implemented!
    return object_type::none;
  }

  object* object::load(const std::string& name)
  {
    assert(false); // Not implemented!
    return nullptr;
  }

  void object::save(object* object)
  {
    assert(false); // Not implemented!
  }

  object* spawn()
  {
    assert(false); // Not implemented!
    return nullptr;
  }

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