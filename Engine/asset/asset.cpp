
#include <assert.h>

#include "asset/asset.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(asset_base, object, "Asset base")
  OBJECT_DEFINE_NOSPAWN(asset_base)

  bool asset_base::load(asset_base* instance, const std::string& name)
  {
    assert(instance);
    instance->file_name = name;
    return true;
  }
}