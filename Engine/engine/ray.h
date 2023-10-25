#pragma once

#include "core/core.h"

#include "vec3.h"

namespace engine
{
  class ENGINE_API ray
  {
  public:
    ray() = default;
    ray(const vec3& origin, const vec3& direction);

    // Returns a point at distance "t" from the origin
    inline vec3 at(float t) const;

  public:
    vec3 origin;
    vec3 direction;
  };
}

