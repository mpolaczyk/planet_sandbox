#pragma once

#include "engine/vec3.h"

#define RAND_SEED_FUNC(seed) rand_pcg(seed)

namespace colors
{
  const vec3 white = vec3(0.73f, .73f, .73f);
  const vec3 grey = vec3(0.6f, 0.6f, 0.6f);
  const vec3 black = vec3(0.0f, 0.0f, 0.0f);
  const vec3 red = vec3(0.65f, 0.05f, 0.05f);
  const vec3 green = vec3(.12f, .45f, .15f);
  const vec3 blue = vec3(0.0f, 0.0f, 1.0f);
  const vec3 white_blue = vec3(0.5f, 0.7f, 1.0f);
  const vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
  const vec3 copper = vec3(0.72f, 0.45f, 0.2f);
  const vec3 steel = vec3(0.44f, 0.47f, 0.49f);
  const vec3 silver = vec3(0.32f, 0.34f, 0.34f);
  const vec3 gold = vec3(1.f, 0.84f, 0.f);

  inline bool is_valid(const vec3& color)
  {
    return color.x <= 1.0f && color.y <= 1.0f && color.z <= 1.0f
      && color.x >= 0.0f && color.y >= 0.0f && color.z >= 0.0f;
  }

  const vec3 all[] =
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
  const int num = 12;
}





namespace random_seed
{
  // Random function compendium: https://www.shadertoy.com/view/XlGcRh
  float rand_iqint1(uint32_t seed);
  float rand_pcg(uint32_t seed);

  vec3 direction(uint32_t seed);
  float normal_distribution(uint32_t seed);
  vec3 cosine_direction(uint32_t seed);
  inline vec3 in_unit_disk(uint32_t seed)
  {
    vec3 dir = math::normalize(vec3(RAND_SEED_FUNC(seed)));
    return dir * RAND_SEED_FUNC(seed);
  }
  inline vec3 unit_in_hemisphere(const vec3& normal, uint32_t seed)
  {
    vec3 dir = direction(seed);
    return dir * math::sign(math::dot(normal, dir));
  }
  
}

namespace random_cache
{
  template<typename T, int N>
  struct cache
  {
    T get();
    void add(T value);
    int32_t len();

  private:
    int last_index = 0;
    std::vector<T> storage;
  };

  static cache<float, 500000> float_cache; // Range: [-1, 1]
  static cache<vec3, 50000> cosine_direction_cache;
  void init();

  float get_float();
  float get_float_0_1();
  float get_float_0_N(float N);
  float get_float_M_N(float M, float N);
  vec3 get_vec3();
  vec3 get_vec3_0_1();
  int32_t get_int_0_N(int32_t N);
  vec3 get_cosine_direction();
  vec3 direction();
  float normal_distribution();
  vec3 cosine_direction();
  vec3 in_sphere(float radius, float distance_squared);
  inline vec3 in_unit_disk()
  {
    vec3 dir = math::normalize(random_cache::get_vec3());
    return dir * random_cache::get_float();
  }
  inline vec3 unit_in_hemisphere(const vec3& normal)
  {
    vec3 dir = math::normalize(random_cache::get_vec3());
    return dir * math::sign(math::dot(dir, normal));
  }
}

namespace tone_mapping
{
  // Tone mapping functions
  // Based on https://64.github.io/tonemapping/
  // radiance vs luminance 
  // - radiance is physical measure (RGB), the amount of light incoming to a point from a single direction
  // - luminance is how humans perceive it, how bright we see something? (scalar) Human perception is non-linear.
  // HDR - High Dynamic Range
  // LDR - Low Dynamic Range
  // TMO - Tone Mapping Operator

  vec3 trivial(const vec3& v);
  vec3 reinhard(const vec3& v);
  vec3 reinhard_extended(const vec3& v, float max_white);
  float luminance(const vec3& v);
  vec3 change_luminance(const vec3& c_in, float l_out);
  vec3 reinhard_extended_luminance(const vec3& v, float max_white_l);
}





namespace obj_helper
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces);
}
class texture;
namespace img_helper
{
  bool load_img(const std::string& file_name, int width, int height, texture* out_texture);
}

