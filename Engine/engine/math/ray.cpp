#include "engine/math/ray.h"

namespace engine
{
  fray::fray(const fvec3& origin, const fvec3& direction)
    : origin(origin), direction(direction)
  {
  }

  fvec3 fray::at(float t) const
  {
    return origin + t * direction;
  }
}
