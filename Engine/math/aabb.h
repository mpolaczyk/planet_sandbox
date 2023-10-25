#pragma once

#include "core/core.h"

#include "math/ray.h"
#include "math/vec3.h"
#include "math/aabb.h"

namespace engine
{
  class ENGINE_API aabb
  {
  public:
    aabb() = default;
    aabb(const vec3& in_minimum, const vec3& in_maximum)
      : minimum(in_minimum), maximum(in_maximum)
    { }

    bool hit(const ray& in_ray, float t_min, float t_max) const;
    bool hit2(const ray& in_ray, float t_min, float t_max) const;

    vec3 minimum;
    vec3 maximum;

    static aabb merge(const aabb& box0, const aabb& box1);
  };
}
