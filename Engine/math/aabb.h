#pragma once

#include "core/core.h"

#include "math/ray.h"
#include "math/vec3.h"
#include "math/aabb.h"

namespace engine
{
  class ENGINE_API faabb
  {
  public:
    faabb() = default;
    faabb(const fvec3& in_minimum, const fvec3& in_maximum)
      : minimum(in_minimum), maximum(in_maximum)
    { }

    bool hit(const fray& in_ray, float t_min, float t_max) const;
    bool hit2(const fray& in_ray, float t_min, float t_max) const;

    fvec3 minimum;
    fvec3 maximum;

    static faabb merge(const faabb& box0, const faabb& box1);
  };
}
