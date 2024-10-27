#include <cassert>

#include "asset/asset.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(aasset_base, oobject, "Asset base")
  OBJECT_DEFINE_NOSPAWN(aasset_base)

  bool aasset_base::load(const std::string& name)
  {
    file_name = name;
    return true;
  }

  void aasset_base::save()
  {
    // TODO this is missing, so far file_name is handled in each child class. This is inconsistent, fix it.
  }
}
