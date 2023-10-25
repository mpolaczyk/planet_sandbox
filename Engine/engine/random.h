#pragma once

#include "core/core.h"

#include "vec3.h"

#include <stdint.h>
#include <vector>

namespace engine
{
  class ENGINE_API random_seed
  {
  public:
    // Random function compendium: https://www.shadertoy.com/view/XlGcRh
    static float rand_iqint1(uint32_t seed);
    static float rand_pcg(uint32_t seed);

    static vec3 direction(uint32_t seed);
    static float normal_distribution(uint32_t seed);
    static vec3 cosine_direction(uint32_t seed);
    static vec3 in_unit_disk(uint32_t seed);
    static vec3 unit_in_hemisphere(const vec3& normal, uint32_t seed);
  };
}

namespace engine
{
  class ENGINE_API random_cache
  {
  public:
    static void init();

    static float get_float();
    static float get_float_0_1();
    static float get_float_0_N(float N);
    static float get_float_M_N(float M, float N);
    static vec3 get_vec3();
    static vec3 get_vec3_0_1();
    static int32_t get_int_0_N(int32_t N);
    static vec3 get_cosine_direction();
    static vec3 direction();
    static float normal_distribution();
    static vec3 cosine_direction();
    static vec3 in_sphere(float radius, float distance_squared);
    static vec3 in_unit_disk();
    static vec3 unit_in_hemisphere(const vec3& normal);
  };
}