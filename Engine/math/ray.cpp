#include "math/ray.h"

namespace engine
{
  ray::ray(const vec3& origin, const vec3& direction)
        : origin(origin), direction(direction)
  {
  }

  vec3 ray::at(float t) const
  {
    return origin + t * direction;
  }
}