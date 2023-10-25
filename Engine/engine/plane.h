#pragma once

#include "core/core.h"

#include "vec3.h"

namespace engine
{
  struct ENGINE_API plane
  {
    vec3 horizontal;            // size horizontal
    vec3 vertical;              // size vertical
    vec3 lower_left_corner;   // world space coordinates
    vec3 get_point(float u, float v) const;
  };
}