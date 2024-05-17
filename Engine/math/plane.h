#pragma once

#include "core/core.h"

#include "math/vec3.h"

namespace engine
{
  struct ENGINE_API fplane
  {
    fvec3 horizontal; // size horizontal
    fvec3 vertical; // size vertical
    fvec3 lower_left_corner; // world space coordinates
    fvec3 get_point(float u, float v) const;
  };
}
