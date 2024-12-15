#pragma once

#include "core/core.h"

#include "engine/math/vec3.h"

namespace engine
{
  class ENGINE_API tone_mapping
  {
    // Tone mapping functions
    // Based on https://64.github.io/tonemapping/
    // radiance vs luminance 
    // - radiance is physical measure (RGB), the amount of light incoming to a point from a single direction
    // - luminance is how humans perceive it, how bright we see something? (scalar) Human perception is non-linear.
    // HDR - High Dynamic Range
    // LDR - Low Dynamic Range
    // TMO - Tone Mapping Operator
  public:
    static fvec3 trivial(const fvec3& v);
    static fvec3 reinhard(const fvec3& v);
    static fvec3 reinhard_extended(const fvec3& v, float max_white);
    static float luminance(const fvec3& v);
    static fvec3 change_luminance(const fvec3& c_in, float l_out);
    static fvec3 reinhard_extended_luminance(const fvec3& v, float max_white_l);
  };
}
