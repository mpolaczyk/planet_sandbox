#include <sstream>
#include <cassert>

#include "object/object.h"

#include "engine/hash.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(oclass_object, oobject, Class object)
  OBJECT_DEFINE_SPAWN(oclass_object)

  OBJECT_DEFINE(oobject, oobject, Object)
  OBJECT_DEFINE_NOSPAWN(oobject)
  OBJECT_DEFINE_VISITOR_BASE(oobject)

  void oobject::set_runtime_id(int id)
  {
    if(runtime_id == -1)
    {
      runtime_id = id;
    }
  }

  int oobject::get_runtime_id() const
  {
    return runtime_id;
  }

  inline uint32_t oobject::get_hash() const
  {
    return fhash::get(static_cast<int64_t>(runtime_id));
  }

  void oobject::destroy()
  {
    REG.destroy(runtime_id);
  }
}
