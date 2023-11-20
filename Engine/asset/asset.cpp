#include "asset/asset.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(asset_base, object)
  OBJECT_DEFINE_NOSPAWN(asset_base)
  OBJECT_DEFINE_NOLOAD(asset_base)
  OBJECT_DEFINE_NOSAVE(asset_base)
}