#include "math/plane.h"

namespace engine
{
  fvec3 fplane::get_point(float u, float v) const
  {
    return lower_left_corner + horizontal * u + v * vertical;
  }
}