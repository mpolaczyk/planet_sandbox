#include "engine/math/aabb.h"
#include "engine/math/vec3.h"
#include "engine/math/math.h"
#include "core/exceptions/floating_point.h"

namespace engine
{
  bool faabb::hit(const fray& in_ray, float t_min, float t_max) const
  {
    if(minimum == fvec3(0.0f) && maximum == fvec3(0.0f))
    {
      return false;
    }
#if 0 // USE_SIMD // commented out, non vectorized is faster in that case
    __m128 minod = _mm_div_ps(_mm_sub_ps(minimum.R128, in_ray.origin.R128), in_ray.direction.R128);
    __m128 maxod = _mm_div_ps(_mm_sub_ps(maximum.R128, in_ray.origin.R128), in_ray.direction.R128);
    vec3 t0 = vec3(_mm_min_ps(minod, maxod));
    vec3 t1 = vec3(_mm_max_ps(minod, maxod));
    for (int i = 0; i < 3; i++)
    {
      t_min = max1(t0[i], t_min);
      t_max = min1(t1[i], t_max);
      if (t_max < t_min)
      {
        return false;
      }
    }
    return true;
#else
    for(int i = 0; i < 3; i++)
    {
      float o = in_ray.origin[i];
      float d_inv = 0.0f;
      {
        ffpe_disabled_scope fpe;
        d_inv = 1.0f / in_ray.direction[i]; // this is allowed to produce 1/0 = inf
      }
      float t0 = fmath::min1((minimum[i] - o) * d_inv, (maximum[i] - o) * d_inv);
      float t1 = fmath::max1((minimum[i] - o) * d_inv, (maximum[i] - o) * d_inv);
      t_min = fmath::max1(t0, t_min);
      t_max = fmath::min1(t1, t_max);
      if(t_max <= t_min)
      {
        return false;
      }
    }
    return true;
#endif
  }

  bool faabb::hit2(const fray& in_ray, float t_min, float t_max) const
  {
    // by Andrew Kensler at Pixar
    for(int a = 0; a < 3; a++)
    {
      float invD = 1.0f / in_ray.direction[a];
      float t0 = (minimum[a] - in_ray.origin[a]) * invD;
      float t1 = (maximum[a] - in_ray.origin[a]) * invD;
      if(invD < 0.0f)
      {
        float temp = t0;
        t0 = t1;
        t1 = temp;
      }
      t_min = t0 > t_min ? t0 : t_min;
      t_max = t1 < t_max ? t1 : t_max;
      if(t_max <= t_min)
      {
        return false;
      }
    }
    return true;
  }

  faabb faabb::merge(const faabb& box0, const faabb& box1)
  {
#if USE_SIMD
    fvec3 corner_min = fvec3(_mm_min_ps(box0.minimum.R128, box1.minimum.R128));
    fvec3 corner_max = fvec3(_mm_min_ps(box0.maximum.R128, box1.maximum.R128));
#else
    vec3 corner_min(min1(box0.minimum.x, box1.minimum.x),
      min1(box0.minimum.y, box1.minimum.y),
      min1(box0.minimum.z, box1.minimum.z));
    vec3 corner_max(max1(box0.maximum.x, box1.maximum.x),
      max1(box0.maximum.y, box1.maximum.y),
      max1(box0.maximum.z, box1.maximum.z));
#endif
    return faabb(corner_min, corner_max);
  }

  fbounding_box fbounding_box::from_min_max(const fvec3& min, const fvec3 max)
  {
    fbounding_box value((min + max) * 0.5f, (min - max) * 0.5f);
    if(fmath::is_almost_zero(value.extents.x)) value.extents.x = fmath::epsilon;
    if(fmath::is_almost_zero(value.extents.y)) value.extents.y = fmath::epsilon;
    if(fmath::is_almost_zero(value.extents.z)) value.extents.z = fmath::epsilon;
    return value;
  }
};
