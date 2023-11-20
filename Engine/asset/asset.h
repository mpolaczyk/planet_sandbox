#pragma once

#include "core/core.h"

#include "object/object.h"

namespace engine
{
  // Base class for all assets, don't instantiate
  class ENGINE_API asset_base : public object
  {
  public:
    OBJECT_DECLARE(asset_base)
  };
}
