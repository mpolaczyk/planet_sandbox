#include <cassert>

#include "asset/asset.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(aasset_base, oobject, "Asset base")
  OBJECT_DEFINE_NOSPAWN(aasset_base)

  bool aasset_base::load(aasset_base* instance, const std::string& name)
  {
    assert(instance);
    instance->file_name = name;
    return true;
  }
}
