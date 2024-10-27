#include "asset/asset.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(aasset_base, oobject, "Asset base")
  OBJECT_DEFINE_NOSPAWN(aasset_base)

  bool aasset_base::load(const std::string& in_name)
  {
    name = in_name;
    return true;
  }
}
