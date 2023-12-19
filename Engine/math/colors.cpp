#include <cassert>

#include "math/colors.h"
#include "math/math.h" 

namespace engine
{
  vec3 tone_mapping::trivial(const vec3& v)
  {
    return math::clamp_vec3(0.0f, 1.0f, v);
  }

  vec3 tone_mapping::reinhard(const vec3& v)
  {
    // Mathematically guarantees to produce [0.0, 1.0]
    // Use with luminance not with RGB radiance
    return v / (1.0f + v);
  }

  vec3 tone_mapping::reinhard_extended(const vec3& v, float max_white)
  {
    // FIX use SIMD
    vec3 numerator = v * (1.0f + (v / vec3(max_white * max_white)));
    return numerator / (1.0f + v);
  }

  float tone_mapping::luminance(const vec3& v)
  {
    float value = math::dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
    assert(value >= 0.0f);
    return value;
  }

  vec3 tone_mapping::change_luminance(const vec3& c_in, float l_out)
  {
    float l_in = luminance(c_in);
    if (l_in == 0.0f)
    {
      return vec3(0, 0, 0);
    }
    return c_in * (l_out / l_in);
  }

  vec3 tone_mapping::reinhard_extended_luminance(const vec3& v, float max_white_l)
  {
    assert(max_white_l > 0.0f);
    float l_old = luminance(v);
    float numerator = l_old * (1.0f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0f + l_old);
    return change_luminance(v, l_new);
  }
}


