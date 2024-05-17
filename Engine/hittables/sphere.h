#pragma once

#include <string>

#include "hittables.h"

#include "math/vec3.h"
#include "math/aabb.h"

namespace engine
{
  class ENGINE_API hsphere : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hsphere, hhittable_base)
    OBJECT_DECLARE_VISITOR

    virtual uint32_t get_hash() const override;

    // Persistent members
    float radius = 0.0f;
  };
}
