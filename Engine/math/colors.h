#pragma once

#include "core/core.h"

#include "math/vec3.h"


namespace engine
{
  class ENGINE_API fcolors
  {
  public:
    static constexpr fvec3 white = fvec3(0.73f, .73f, .73f);
    static constexpr fvec3 grey = fvec3(0.6f, 0.6f, 0.6f);
    static constexpr fvec3 black = fvec3(0.0f, 0.0f, 0.0f);
    static constexpr fvec3 red = fvec3(0.65f, 0.05f, 0.05f);
    static constexpr fvec3 green = fvec3(.12f, .45f, .15f);
    static constexpr fvec3 blue = fvec3(0.0f, 0.0f, 1.0f);
    static constexpr fvec3 white_blue = fvec3(0.5f, 0.7f, 1.0f);
    static constexpr fvec3 yellow = fvec3(1.0f, 1.0f, 0.0f);
    static constexpr fvec3 copper = fvec3(0.72f, 0.45f, 0.2f);
    static constexpr fvec3 steel = fvec3(0.44f, 0.47f, 0.49f);
    static constexpr fvec3 silver = fvec3(0.32f, 0.34f, 0.34f);
    static constexpr fvec3 gold = fvec3(1.f, 0.84f, 0.f);

    static constexpr fvec3 all[] =
    {
      white,
      grey,
      black,
      red,
      green,
      blue,
      white_blue,
      yellow,
      copper,
      steel,
      silver,
      gold
    };

    static constexpr int num = 12;

    static bool is_valid(const fvec3& color)
    {
      return color.x <= 1.0f && color.y <= 1.0f && color.z <= 1.0f
        && color.x >= 0.0f && color.y >= 0.0f && color.z >= 0.0f;
    }
  };
}

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


