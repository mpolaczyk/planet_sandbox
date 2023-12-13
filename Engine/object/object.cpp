
#include <assert.h>
#include <sstream>  

#include "object.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(class_object, object, Class object)
  OBJECT_DEFINE_SPAWN(class_object)

  OBJECT_DEFINE(object, object, Object)
  OBJECT_DEFINE_NOSPAWN(object)

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

}