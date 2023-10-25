#pragma once

#include "core/core.h"

#include "math/vec3.h"
#include "math/math.h"

namespace engine
{
  // OrthoNormal Base
  struct ENGINE_API onb
  {
    vec3 local(const vec3& a) const;
    void build_from_w(const vec3& in_w);

    vec3 u, v, w;
  };
}