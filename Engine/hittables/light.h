#pragma once

#include <string>

#include "engine/hittable.h"
#include "engine/renderer/aligned_structs.h"

namespace engine
{
  class ENGINE_API hlight : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hlight, hhittable_base)
    OBJECT_DECLARE_VISITOR

    virtual uint32_t get_hash() const override;

    // Persistent members
    flight_properties properties;
  };
}
