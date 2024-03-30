#pragma once

#include "core/core.h"

#include "math/vec3.h"
#include "math/math.h"

namespace engine
{
  // OrthoNormal Base
  struct ENGINE_API fonb
  {
    fvec3 local(const fvec3& a) const;
    void build_from_w(const fvec3& in_w);

    fvec3 u, v, w;
  };
}