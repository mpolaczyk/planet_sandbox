#pragma once

#include "core/core.h"

#include "math/vec3.h"

namespace engine
{
  struct ENGINE_API fray
  {
  public:
    CTOR_DEFAULT(fray)
    fray(const fvec3& origin, const fvec3& direction);

    // Returns a point at distance "t" from the origin
    inline fvec3 at(float t) const;

  public:
    fvec3 origin;
    fvec3 direction;
  };
}
