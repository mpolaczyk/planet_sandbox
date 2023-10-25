#pragma once

#include <corecrt_math.h>
#include <assert.h>
#include <stdint.h>

#include "math/math.h"
#include "profile/stats.h"

namespace engine
{
  float math::sign(float value)
  {
    return value >= 0.0f ? 1.0f : -1.0f;  //  Assume 0 is positive
  }

  float math::degrees_to_radians(float degrees)
  {
    return degrees * math::pi / 180.0f;
  }

  bool math::is_almost_zero(float value)
  {
    return value <= epsilon && value >= -epsilon;
  }

  bool math::is_almost_equal(float a, float b)
  {
    return fabs(a - b) <= epsilon;
  }

  float math::inv_sqrt(float x)
  {
    // Fast inverse square root
    float xhalf = 0.5f * x;
    int32_t i = reinterpret_cast<int32_t&>(x);  // store floating-point bits in integer
    i = 0x5f3759df - (i >> 1);                  // initial guess for Newton's method
    x = reinterpret_cast<float&>(i);            // convert new bits into float
    x = x * (1.5f - xhalf * x * x);             // One round of Newton's method
    return x;
  }

  float math::min1(float a, float b)
  {
    return a < b ? a : b;
  }

  float math::max1(float a, float b)
  {
    return a < b ? b : a;
  }

  float math::clamp(float f, float a, float b)
  {
    return  min1(b, max1(a, f));
  }

  float math::smoothstep(float a, float b, float x)
  {
    // https://thebookofshaders.com/glossary/?search=smoothstep
    float t = clamp(0.0f, 1.0f, (x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
  }

  float math::lerp_float(float a, float b, float f)
  {
    return a + f * (b - a);
  }

  float math::reflectance(float cosine, float ref_idx)
  {
    // Use Schlick's approximation for reflectance.
    float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * (float)pow((1.0f - cosine), 5.0f);
  }

  bool math::flip_normal_if_front_face(const vec3& in_ray_direction, const vec3& in_outward_normal, vec3& out_normal)
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

  vec3 math::reflect(const vec3& vec, const vec3& normal)
  {
    return vec - 2 * dot(vec, normal) * normal;
  }

  vec3 math::refract(const vec3& v, const vec3& n, float etai_over_etat)
  {
    float cos_theta = (float)fmin(dot(-v, n), 1.0f);
    vec3 r_out_perpendicular = etai_over_etat * (v + cos_theta * n);
    vec3 r_out_parallel = -(float)sqrt(fabs(1.0f - math::length_squared(r_out_perpendicular))) * n;
    return r_out_perpendicular + r_out_parallel;
  }

  vec3 math::lerp_vec3(const vec3& a, const vec3& b, float f)
  {
    return vec3(math::lerp_float(a.x, b.x, f), math::lerp_float(a.y, b.y, f), math::lerp_float(a.z, b.z, f));
  }

  vec3 math::clamp_vec3(float a, float b, const vec3& f)
  {
    vec3 ans;
    ans.x = math::clamp(f.x, a, b);
    ans.y = math::clamp(f.y, a, b);
    ans.z = math::clamp(f.z, a, b);
    return ans;
  }

  bool math::ray_triangle(const ray& in_ray, float t_min, float t_max, const triangle_face* in_triangle, hit_record& out_hit, bool drop_backface)
  {
    assert(in_triangle != nullptr);
    stats::inc_ray_triangle_intersection();

    // https://graphicscodex.courses.nvidia.com/app.html?page=_rn_rayCst "4. Ray-Triangle Intersection"

    // Vertices
    const vec3& V0 = in_triangle->vertices[0];
    const vec3& V1 = in_triangle->vertices[1];
    const vec3& V2 = in_triangle->vertices[2];

    // Edge vectors
    const vec3& E1 = V1 - V0;
    const vec3& E2 = V2 - V0;

    // Face normal
    vec3 n = normalize(cross(E1, E2));

    // Ray origin and direction
    const vec3& P = in_ray.origin;
    const vec3& w = in_ray.direction;

    // Detect backface
    // !!!! it works but should be the opposite! are faces left or right oriented?
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
    vec3 q = cross(w, E2);
    float a = dot(E1, q);

    // Ray parallel or close to the limit of precision?
    if (fabsf(a) <= small_number) return false;

    // ?
    const vec3& s = (P - V0) / a;
    const vec3& r = cross(s, E1);

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
    vec3 barycentric(b[2], b[0], b[1]);
    // this does not work, TODO translate normals?
    // Vertex normals
    //out_hit.normal = normalize(barycentric[0] * in_triangle->normals[0] + barycentric[1] * in_triangle->normals[1] + barycentric[2] * in_triangle->normals[2]);
    // Face normals
    out_hit.normal = n;

    const vec3& uv = barycentric[0] * in_triangle->UVs[0] + barycentric[1] * in_triangle->UVs[1] + barycentric[2] * in_triangle->UVs[2];
    out_hit.u = uv.x;
    out_hit.v = uv.y;

    return true;
  }

  bool math::is_near_zero(const vec3& value)
  {
    return (fabs(value[0]) < very_small_number) && (fabs(value[1]) < very_small_number) && (fabs(value[2]) < very_small_number);
  }

  bool math::is_zero(const vec3& value)
  {
    return value.x == 0.0f && value.y == 0.0f && value.z == 0.0f;
  }

  float math::dot(const vec3& u, const vec3& v)
  {
#if USE_SIMD 
    __m128 a = _mm_mul_ps(u.R128, v.R128);
    a = _mm_hadd_ps(a, a);
    return _mm_cvtss_f32(_mm_hadd_ps(a, a));
#else
    return u.x * v.x + u.y * v.y + u.z * v.z;
#endif
  }

  vec3 math::cross(const vec3& u, const vec3& v)
  {
    return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
  }

  vec3 math::normalize(const vec3& v)
  {
#if USE_SIMD 
    __m128 a = _mm_mul_ps(v.R128, v.R128);
    a = _mm_hadd_ps(a, a);
    return vec3(_mm_div_ps(v.R128, _mm_sqrt_ps(_mm_hadd_ps(a, a))));
#else
    return v / length(v);
#endif
  }
  float math::length(const vec3& v)
  {
#if USE_SIMD 
    __m128 a = _mm_mul_ps(v.R128, v.R128);
    a = _mm_hadd_ps(a, a);
    return _mm_cvtss_f32(_mm_sqrt_ps(_mm_hadd_ps(a, a)));
#else
    return std::sqrt(length_squared(v));
#endif
  }
  float math::length_squared(const vec3& v)
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

  vec3 math::rotate_yaw(const vec3& u, float yaw)
  {
    float s = sinf(yaw);
    float c = cosf(yaw);
    return vec3(c * u.x - s * u.y, s * u.x + c * u.y, u.z);
  }

  vec3 math::rotate_pitch(const vec3& u, float pitch)
  {
    float s = sinf(pitch);
    float c = cosf(pitch);
    return vec3(u.x, c * u.y - s * u.z, s * u.y + c * u.z);
  }

  vec3 math::rotate_roll(const vec3& u, float roll)
  {
    float s = sinf(roll);
    float c = cosf(roll);
    return vec3(c * u.x - s * u.z, u.y, s * u.x + c * u.z);
  }

  vec3 math::min3(const vec3& a, const vec3& b)
  {
    return vec3(min1(a[0], b[0]), min1(a[1], b[1]), min1(a[2], b[2]));
  }

  vec3 math::max3(const vec3& a, const vec3& b)
  {
    return vec3(max1(a[0], b[0]), max1(a[1], b[1]), max1(a[2], b[2]));
  }

  void math::get_sphere_uv(const vec3& p, float& out_u, float& out_v)
  {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
    vec3 pp = normalize(p); // normalize to get sensible values for acos, otherwise floating point exceptions will happen
    float theta = (float)acos(-pp.y);
    float phi = (float)atan2(-pp.z, pp.x) + pi;
    out_u = phi / (2.0f * pi);
    out_v = theta / pi;
  }
}