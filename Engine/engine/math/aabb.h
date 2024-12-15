#pragma once

#include "core/core.h"

#include "engine/math/ray.h"
#include "engine/math/vec3.h"

namespace engine
{
  struct ENGINE_API faabb
  {
  public:
    CTOR_DEFAULT(faabb)

    faabb(const fvec3& in_minimum, const fvec3& in_maximum)
      : minimum(in_minimum), maximum(in_maximum)
    {
    }

    bool hit(const fray& in_ray, float t_min, float t_max) const;
    bool hit2(const fray& in_ray, float t_min, float t_max) const;

    fvec3 minimum;
    fvec3 maximum;

    static faabb merge(const faabb& box0, const faabb& box1);
  };

  struct ENGINE_API fbounding_box
  {
    CTOR_DEFAULT(fbounding_box)

    fbounding_box(const fvec3& in_center, const fvec3& in_extents)
      : center(in_center), extents(in_extents)
    {}

    static fbounding_box from_min_max(const fvec3& min, const fvec3 max);
    
    fvec3 center;
    fvec3 extents;
  };
}
