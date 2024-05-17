#include <cassert>

#include "math/colors.h"
#include "math/math.h"

namespace engine
{
  fvec3 tone_mapping::trivial(const fvec3& v)
  {
    return fmath::clamp_vec3(0.0f, 1.0f, v);
  }

  fvec3 tone_mapping::reinhard(const fvec3& v)
  {
    // Mathematically guarantees to produce [0.0, 1.0]
    // Use with luminance not with RGB radiance
    return v / (1.0f + v);
  }

  fvec3 tone_mapping::reinhard_extended(const fvec3& v, float max_white)
  {
    // FIX use SIMD
    fvec3 numerator = v * (1.0f + (v / fvec3(max_white * max_white)));
    return numerator / (1.0f + v);
  }

  float tone_mapping::luminance(const fvec3& v)
  {
    float value = fmath::dot(v, fvec3(0.2126f, 0.7152f, 0.0722f));
    assert(value >= 0.0f);
    return value;
  }

  fvec3 tone_mapping::change_luminance(const fvec3& c_in, float l_out)
  {
    float l_in = luminance(c_in);
    if(l_in == 0.0f)
    {
      return fvec3(0, 0, 0);
    }
    return c_in * (l_out / l_in);
  }

  fvec3 tone_mapping::reinhard_extended_luminance(const fvec3& v, float max_white_l)
  {
    assert(max_white_l > 0.0f);
    float l_old = luminance(v);
    float numerator = l_old * (1.0f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0f + l_old);
    return change_luminance(v, l_new);
  }
}
