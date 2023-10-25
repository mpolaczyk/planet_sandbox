#include "math/plane.h"

namespace engine
{
  vec3 plane::get_point(float u, float v) const
  {
    return lower_left_corner + horizontal * u + v * vertical;
  }
}