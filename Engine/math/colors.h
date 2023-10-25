#pragma once

#include "core/core.h"

#include "math/vec3.h"


namespace engine
{
  class ENGINE_API colors
  {
  public:
    static constexpr vec3 white = vec3(0.73f, .73f, .73f);
    static constexpr vec3 grey = vec3(0.6f, 0.6f, 0.6f);
    static constexpr vec3 black = vec3(0.0f, 0.0f, 0.0f);
    static constexpr vec3 red = vec3(0.65f, 0.05f, 0.05f);
    static constexpr vec3 green = vec3(.12f, .45f, .15f);
    static constexpr vec3 blue = vec3(0.0f, 0.0f, 1.0f);
    static constexpr vec3 white_blue = vec3(0.5f, 0.7f, 1.0f);
    static constexpr vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
    static constexpr vec3 copper = vec3(0.72f, 0.45f, 0.2f);
    static constexpr vec3 steel = vec3(0.44f, 0.47f, 0.49f);
    static constexpr vec3 silver = vec3(0.32f, 0.34f, 0.34f);
    static constexpr vec3 gold = vec3(1.f, 0.84f, 0.f);

    static constexpr vec3 all[] =
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

    static bool is_valid(const vec3& color)
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
    static vec3 trivial(const vec3& v);
    static vec3 reinhard(const vec3& v);
    static vec3 reinhard_extended(const vec3& v, float max_white);
    static float luminance(const vec3& v);
    static vec3 change_luminance(const vec3& c_in, float l_out);
    static vec3 reinhard_extended_luminance(const vec3& v, float max_white_l);
  };
}


