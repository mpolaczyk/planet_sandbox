#include "engine/math/random.h"
#include "engine/math/math.h"
#include "engine/time.h"

#include <random>
#include <intsafe.h>
#include <cassert>

namespace engine
{
  float frandom_seed::rand_iqint1(uint32_t seed)
  {
    static uint32_t last = 0;
    uint32_t state = seed + last; // Seed can be the same for multiple calls, we need to rotate it
    state = (state << 13U) ^ state;
    state = state * (state * state * 15731U + 789221U) + 1376312589U;
    last = state;
    return (float)state / (float)UINT_MAX; // [0.0f, 1.0f]
  }

  float frandom_seed::rand_pcg(uint32_t seed)
  {
    static uint32_t last = 0;
    uint32_t state = seed + last; // Seed can be the same for multiple calls, we need to rotate it
    state = state * 747796405U + 2891336453U;
    uint32_t word = ((state >> ((state >> 28U) + 4U)) ^ state) * 277803737U;
    uint32_t result = ((word >> 22U) ^ word);
    last = result;
    return (float)result / (float)UINT_MAX; // [0.0f, 1.0f]
  }

  fvec3 frandom_seed::direction(uint32_t seed)
  {
    float x = normal_distribution(seed);
    float y = normal_distribution(seed);
    float z = normal_distribution(seed);
    return fmath::normalize(fvec3(x, y, z));
  }

  float frandom_seed::normal_distribution(uint32_t seed)
  {
    float theta = 2.0f * fmath::pi * RAND_SEED_FUNC(seed);
    float rho = sqrt(-2.0f * log(RAND_SEED_FUNC(seed)));
    assert(isfinite(rho));
    return rho * cos(theta); // [-1.0f, 1.0f]
  }

  fvec3 frandom_seed::cosine_direction(uint32_t seed)
  {
    // https://psgraphics.blogspot.com/2013/11/random-directions-with-cosine.html
    // Cosine distribution around positive z axis
    float r1 = RAND_SEED_FUNC(seed);
    float r2 = RAND_SEED_FUNC(seed);
    //float phi = 2 * pi * r1;
    float r = 0.5;
    float x = cos(2 * fmath::pi * r1) * sqrt(r);
    float y = sin(2 * fmath::pi * r2) * sqrt(r);
    float z = sqrt(1 - r);
    return fvec3(x, y, z);
  }

  fvec3 frandom_seed::in_unit_disk(uint32_t seed)
  {
    fvec3 dir = fmath::normalize(fvec3(RAND_SEED_FUNC(seed)));
    return dir * RAND_SEED_FUNC(seed);
  }

  fvec3 frandom_seed::unit_in_hemisphere(const fvec3& normal, uint32_t seed)
  {
    fvec3 dir = direction(seed);
    return dir * fmath::sign(fmath::dot(normal, dir));
  }
}

namespace engine
{
  template <typename T, int N>
  struct cache
  {
    T get()
    {
      assert(last_index >= 0 && last_index < N);
      last_index = (last_index + 1) % N;
      return storage[last_index];
    }

    void add(T value)
    {
      storage.push_back(value);
    }

    int32_t len()
    {
      return N;
    }

  private:
    int last_index = 0;
    std::vector<T> storage;
  };

  static cache<float, 500000> float_cache; // Range: [-1, 1]
  static cache<fvec3, 50000> cosine_direction_cache;

  void frandom_cache::init()
  {
    // Fill float cache
    std::uniform_real_distribution<float> distribution;
    distribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    std::mt19937 generator(static_cast<uint32_t>(ftime::get_now_us()));
    for(int s = 0; s < float_cache.len(); s++)
    {
      float_cache.add(distribution(generator));
    }

    // Fill cosine direction cache
    for(int s = 0; s < cosine_direction_cache.len(); s++)
    {
      cosine_direction_cache.add(cosine_direction());
    }
  }

  float frandom_cache::get_float()
  {
    return float_cache.get();
  }

  float frandom_cache::get_float_0_1()
  {
    return fabs(float_cache.get());
  }

  float frandom_cache::get_float_0_N(float N)
  {
    return fabs(float_cache.get()) * N;
  }

  float frandom_cache::get_float_M_N(float M, float N)
  {
    if(M < N)
    {
      return M + fabs(float_cache.get()) * (N - M);
    }
    return N + fabs(float_cache.get()) * (M - N);
  }

  fvec3 frandom_cache::get_vec3() // [-1,1]
  {
    return fvec3(float_cache.get(), float_cache.get(), float_cache.get());
  }

  fvec3 frandom_cache::get_vec3_0_1()
  {
    return fvec3(fabs(float_cache.get()), fabs(float_cache.get()), fabs(float_cache.get()));
  }

  int frandom_cache::get_int_0_N(int N)
  {
    return (int)round(get_float_0_1() * N);
  }

  fvec3 frandom_cache::get_cosine_direction()
  {
    return cosine_direction_cache.get();
  }

  fvec3 frandom_cache::direction()
  {
    float x = normal_distribution();
    float y = normal_distribution();
    float z = normal_distribution();
    return fmath::normalize(fvec3(x, y, z));
  }

  fvec3 frandom_cache::cosine_direction()
  {
    // https://psgraphics.blogspot.com/2013/11/random-directions-with-cosine.html
    // Cosine distribution around positive z axis
    float r1 = get_float_0_1();
    float r2 = get_float_0_1();
    //float phi = 2 * pi * r1;
    float r = 0.5;
    float x = cos(2 * fmath::pi * r1) * sqrt(r);
    float y = sin(2 * fmath::pi * r2) * sqrt(r);
    float z = sqrt(1 - r);
    return fvec3(x, y, z);
  }

  fvec3 frandom_cache::in_sphere(float radius, float distance_squared)
  {
    float r1 = get_float_0_1();
    float r2 = get_float_0_1();
    float z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

    float phi = 2 * fmath::pi * r1;
    float x = cos(phi) * sqrt(1 - z * z);
    float y = sin(phi) * sqrt(1 - z * z);

    return fvec3(x, y, z);
  }

  float frandom_cache::normal_distribution()
  {
    float theta = 2.0f * fmath::pi * get_float_0_1();
    float rho = sqrt(-2.0f * log(get_float_0_1()));
    assert(isfinite(rho));
    return rho * cos(theta); // [-1.0f, 1.0f]
  }

  fvec3 frandom_cache::in_unit_disk()
  {
    fvec3 dir = fmath::normalize(get_vec3());
    return dir * get_float();
  }

  fvec3 frandom_cache::unit_in_hemisphere(const fvec3& normal)
  {
    fvec3 dir = fmath::normalize(get_vec3());
    return dir * fmath::sign(fmath::dot(dir, normal));
  }
}
