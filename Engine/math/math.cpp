#pragma once

#include <corecrt_math.h>
#include <cassert>
#include <stdint.h>
#include <DirectXMath.h>

#include "math/math.h"
#include "profile/stats.h"

namespace engine
{
  float fmath::sign(float value)
  {
    return value >= 0.0f ? 1.0f : -1.0f;  //  Assume 0 is positive
  }

  float fmath::degrees_to_radians(float degrees)
  {
    return degrees * fmath::pi / 180.0f;
  }

  bool fmath::is_almost_zero(float value)
  {
    return value <= epsilon && value >= -epsilon;
  }

  bool fmath::is_almost_equal(float a, float b)
  {
    return fabs(a - b) <= epsilon;
  }

  float fmath::inv_sqrt(float x)
  {
    // Fast inverse square root
    float xhalf = 0.5f * x;
    int32_t i = reinterpret_cast<int32_t&>(x);  // store floating-point bits in integer
    i = 0x5f3759df - (i >> 1);                  // initial guess for Newton's method
    x = reinterpret_cast<float&>(i);            // convert new bits into float
    x = x * (1.5f - xhalf * x * x);             // One round of Newton's method
    return x;
  }

  float fmath::min1(float a, float b)
  {
    return a < b ? a : b;
  }

  float fmath::max1(float a, float b)
  {
    return a < b ? b : a;
  }

  float fmath::clamp(float f, float a, float b)
  {
    return  min1(b, max1(a, f));
  }

  float fmath::smoothstep(float a, float b, float x)
  {
    // https://thebookofshaders.com/glossary/?search=smoothstep
    float t = clamp(0.0f, 1.0f, (x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
  }

  float fmath::lerp_float(float a, float b, float f)
  {
    return a + f * (b - a);
  }

  float fmath::reflectance(float cosine, float ref_idx)
  {
    // Use Schlick's approximation for reflectance.
    float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * (float)pow((1.0f - cosine), 5.0f);
  }

  bool fmath::flip_normal_if_front_face(const fvec3& in_ray_direction, const fvec3& in_outward_normal, fvec3& out_normal)
  {
    if (dot(in_ray_direction, in_outward_normal) < 0)
    {
      // Ray is inside
      out_normal = in_outward_normal;
      return true;
    }
    else
    {
      // Ray is outside
      out_normal = -in_outward_normal;
      return false;
    }
  }

  fvec3 fmath::reflect(const fvec3& vec, const fvec3& normal)
  {
    return vec - 2 * dot(vec, normal) * normal;
  }

  fvec3 fmath::refract(const fvec3& v, const fvec3& n, float etai_over_etat)
  {
    float cos_theta = (float)fmin(dot(-v, n), 1.0f);
    fvec3 r_out_perpendicular = etai_over_etat * (v + cos_theta * n);
    fvec3 r_out_parallel = -(float)sqrt(fabs(1.0f - fmath::length_squared(r_out_perpendicular))) * n;
    return r_out_perpendicular + r_out_parallel;
  }

  fvec3 fmath::lerp_vec3(const fvec3& a, const fvec3& b, float f)
  {
    return fvec3(fmath::lerp_float(a.x, b.x, f), fmath::lerp_float(a.y, b.y, f), fmath::lerp_float(a.z, b.z, f));
  }

  fvec3 fmath::clamp_vec3(float a, float b, const fvec3& f)
  {
    fvec3 ans;
    ans.x = fmath::clamp(f.x, a, b);
    ans.y = fmath::clamp(f.y, a, b);
    ans.z = fmath::clamp(f.z, a, b);
    return ans;
  }

  bool fmath::ray_triangle(const fray& in_ray, float t_max, const ftriangle_face* in_triangle, fhit_record& out_hit, bool drop_backface)
  {
    // FIX use SIMD
    assert(in_triangle != nullptr);
    fstats::inc_ray_triangle_intersection();

    // https://graphicscodex.courses.nvidia.com/app.html?page=_rn_rayCst "4. Ray-Triangle Intersection"

    // Vertices
    const fvec3& V0 = in_triangle->vertices[0];
    const fvec3& V1 = in_triangle->vertices[1];
    const fvec3& V2 = in_triangle->vertices[2];

    // Edge vectors
    const fvec3& E1 = V1 - V0;
    const fvec3& E2 = V2 - V0;

    // Face normal
    fvec3 n = normalize(cross(E1, E2));

    // Ray origin and direction
    const fvec3& P = in_ray.origin;
    const fvec3& w = in_ray.direction;

    // Detect backface
    // !!!! it works but should be the opposite! are faces left or right oriented?
    // FIX use branchless
    if ((dot(n, w) < 0))
    {
      out_hit.front_face = true;
    }
    else
    {
      out_hit.front_face = false;
      n = n * -1;
      if (drop_backface) return false;
    }

    // Plane intersection what is q and a?
    fvec3 q = cross(w, E2);
    float a = dot(E1, q);

    // Ray parallel or close to the limit of precision?
    if (fabsf(a) <= small_number) return false;

    // ?
    const fvec3& s = (P - V0) / a;
    const fvec3& r = cross(s, E1);

    // Barycentric coordinates
    float b[3];
    b[0] = dot(s, q);
    b[1] = dot(r, w);
    b[2] = 1.0f - (b[0] + b[1]);

    // Intersection outside of triangle?
    if ((b[0] < 0.0f) || (b[1] < 0.0f) || (b[2] < 0.0f)) return false;

    // Distance to intersection
    float t = dot(E2, r);

    // Intersection outside of ray range?
    if (t < t_min || t > t_max) return false;

    // Intersected inside triangle
    out_hit.t = t;
    out_hit.p = in_ray.at(t);

    // ?
    fvec3 barycentric(b[2], b[0], b[1]);
    // this does not work, TODO translate normals?
    // Vertex normals
    //out_hit.normal = normalize(barycentric[0] * in_triangle->normals[0] + barycentric[1] * in_triangle->normals[1] + barycentric[2] * in_triangle->normals[2]);
    // Face normals
    out_hit.normal = n;

    const fvec3& uv = barycentric[0] * in_triangle->UVs[0] + barycentric[1] * in_triangle->UVs[1] + barycentric[2] * in_triangle->UVs[2];
    out_hit.u = uv.x;
    out_hit.v = uv.y;

    return true;
  }

  bool fmath::is_near_zero(const fvec3& value)
  {
    return (fabs(value[0]) < very_small_number) && (fabs(value[1]) < very_small_number) && (fabs(value[2]) < very_small_number);
  }

  bool fmath::is_zero(const fvec3& value)
  {
    return value.x == 0.0f && value.y == 0.0f && value.z == 0.0f;
  }

  float fmath::dot(const fvec3& u, const fvec3& v)
  {
#if USE_SIMD 
    __m128 a = _mm_mul_ps(u.R128, v.R128);
    a = _mm_hadd_ps(a, a);
    return _mm_cvtss_f32(_mm_hadd_ps(a, a));
#else
    return u.x * v.x + u.y * v.y + u.z * v.z;
#endif
  }

  // https://geometrian.com/programming/tutorials/cross-product/index.php
  fvec3 fmath::cross(const fvec3& u, const fvec3& v)
  {
#if USE_SIMD
    __m128 tmp0 = _mm_shuffle_ps(u.R128, u.R128, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 tmp1 = _mm_shuffle_ps(v.R128, v.R128, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 tmp2 = _mm_mul_ps(tmp0, v.R128);
    __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
    __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
    return fvec3(_mm_sub_ps(tmp3, tmp4));
#else
    return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
#endif
  }

  fvec3 fmath::normalize(const fvec3& v)
  {
#if USE_SIMD 
    __m128 a = _mm_mul_ps(v.R128, v.R128);
    a = _mm_hadd_ps(a, a);
    return fvec3(_mm_div_ps(v.R128, _mm_sqrt_ps(_mm_hadd_ps(a, a))));
#else
    return v / length(v);
#endif
  }
  float fmath::length(const fvec3& v)
  {
#if USE_SIMD 
    __m128 a = _mm_mul_ps(v.R128, v.R128);
    a = _mm_hadd_ps(a, a);
    return _mm_cvtss_f32(_mm_sqrt_ps(_mm_hadd_ps(a, a)));
#else
    return std::sqrt(length_squared(v));
#endif
  }
  float fmath::length_squared(const fvec3& v)
  {
    // commented out, non vectorized is faster in that case
    //#if USE_SIMD 
    //    __m128 a = _mm_mul_ps(v.R128, v.R128);
    //    a = _mm_hadd_ps(a, a);
    //    return _mm_cvtss_f32(_mm_hadd_ps(a,a));vv
    //#else
    return v.x * v.x + v.y * v.y + v.z * v.z;
    //#endif
  }

  fvec3 fmath::rotate_yaw(const fvec3& u, float yaw)
  {
    float s = sinf(yaw);
    float c = cosf(yaw);
    return fvec3(c * u.x - s * u.y, s * u.x + c * u.y, u.z);
  }

  fvec3 fmath::rotate_pitch(const fvec3& u, float pitch)
  {
    float s = sinf(pitch);
    float c = cosf(pitch);
    return fvec3(u.x, c * u.y - s * u.z, s * u.y + c * u.z);
  }

  fvec3 fmath::rotate_roll(const fvec3& u, float roll)
  {
    float s = sinf(roll);
    float c = cosf(roll);
    return fvec3(c * u.x - s * u.z, u.y, s * u.x + c * u.z);
  }

  fvec3 fmath::rpy_to_direction(const fvec3& rpy)
  {
    fvec3 direction;
    direction.x = cosf(rpy.z) * cosf(rpy.x);
    direction.y = sinf(rpy.z) * cosf(rpy.x);
    direction.z = sinf(rpy.x);
    return direction;
  }

  fvec3 fmath::min3(const fvec3& a, const fvec3& b)
  {
    return fvec3(min1(a[0], b[0]), min1(a[1], b[1]), min1(a[2], b[2]));
  }

  fvec3 fmath::max3(const fvec3& a, const fvec3& b)
  {
    return fvec3(max1(a[0], b[0]), max1(a[1], b[1]), max1(a[2], b[2]));
  }

  void fmath::get_sphere_uv(const fvec3& p, float& out_u, float& out_v)
  {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
    fvec3 pp = normalize(p); // normalize to get sensible values for acos, otherwise floating point exceptions will happen
    float theta = (float)acos(-pp.y);
    float phi = (float)atan2(-pp.z, pp.x) + pi;
    out_u = phi / (2.0f * pi);
    out_v = theta / pi;
  }

  fvec3 fmath::to_vec3(const DirectX::XMFLOAT4& a)
  {
    return fvec3(a.x, a.y, a.z);
  }

  DirectX::XMFLOAT4 fmath::to_xmfloat4(const fvec3& a)
  {
    return DirectX::XMFLOAT4(a.x, a.y, a.z, 1.0f);
  }
}