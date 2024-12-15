#pragma once

#include "core/core.h"

#include "engine/math/vec3.h"

#include <stdint.h>

namespace engine
{
  class ENGINE_API frandom_seed
  {
  public:
    // Random function compendium: https://www.shadertoy.com/view/XlGcRh
    static float rand_iqint1(uint32_t seed);
    static float rand_pcg(uint32_t seed);

    static fvec3 direction(uint32_t seed);
    static float normal_distribution(uint32_t seed);
    static fvec3 cosine_direction(uint32_t seed);
    static fvec3 in_unit_disk(uint32_t seed);
    static fvec3 unit_in_hemisphere(const fvec3& normal, uint32_t seed);
  };
}

namespace engine
{
  class ENGINE_API frandom_cache
  {
  public:
    static void init();

    static float get_float();
    static float get_float_0_1();
    static float get_float_0_N(float N);
    static float get_float_M_N(float M, float N);
    static fvec3 get_vec3();
    static fvec3 get_vec3_0_1();
    static int32_t get_int_0_N(int32_t N);
    static fvec3 get_cosine_direction();
    static fvec3 direction();
    static float normal_distribution();
    static fvec3 cosine_direction();
    static fvec3 in_sphere(float radius, float distance_squared);
    static fvec3 in_unit_disk();
    static fvec3 unit_in_hemisphere(const fvec3& normal);
  };
}
