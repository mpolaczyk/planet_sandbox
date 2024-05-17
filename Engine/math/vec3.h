#pragma once

#include "core/core.h"

#if USE_SIMD
#include <intrin.h> // SSE3 required
#endif

namespace engine
{
  ALIGN(16) struct ENGINE_API fvec3
  {
  public:
    explicit fvec3() = default;

    explicit constexpr fvec3(float in_x, float in_y, float in_z) : e{in_x, in_y, in_z}
    {
    }
#if USE_SIMD
    explicit fvec3(float f) { R128 = _mm_set_ps1(f); }
    explicit fvec3(const __m128& r128) { R128 = r128; }
#else
    vec3(float f) { x = f; y = f; z = f; }
#endif

    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }

    fvec3 operator -() const { return fvec3(-x, -y, -z); }

#if USE_SIMD
    fvec3& operator +=(const fvec3& v)
    {
      R128 = _mm_add_ps(R128, v.R128);
      return *this;
    }

    fvec3& operator -=(const fvec3& v)
    {
      R128 = _mm_sub_ps(R128, v.R128);
      return *this;
    }

    fvec3& operator *=(const fvec3& v)
    {
      R128 = _mm_mul_ps(R128, v.R128);
      return *this;
    }

    fvec3& operator /=(const fvec3& v)
    {
      R128 = _mm_div_ps(R128, v.R128);
      return *this;
    }

    fvec3& operator +=(float t)
    {
      R128 = _mm_add_ps(R128, _mm_set_ps1(t));
      return *this;
    }

    fvec3& operator -=(float t)
    {
      R128 = _mm_sub_ps(R128, _mm_set_ps1(t));
      return *this;
    }

    fvec3& operator *=(float t)
    {
      R128 = _mm_mul_ps(R128, _mm_set_ps1(t));
      return *this;
    }

    fvec3& operator /=(float t)
    {
      R128 = _mm_div_ps(R128, _mm_set_ps1(t));
      return *this;
    }
#else
    vec3& operator += (const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    vec3& operator -= (const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    vec3& operator *= (const vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    vec3& operator /= (const vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
    vec3& operator += (float t) { return *this += t; }
    vec3& operator -= (float t) { return *this -= t; }
    vec3& operator *= (float t) { return *this *= t; }
    vec3& operator /= (float t) { return *this /= t; }
#endif
    bool operator==(const fvec3& v) { return x == v.x && y == v.y && z == v.z; }

  public:
    union
    {
      float e[4] = {0.0f, 0.0f, 0.0f, 0.0f};

      struct
      {
        float x, y, z, padding;
      };

      __m128 R128;
    };
  };

#if USE_SIMD
  inline fvec3 operator +(const fvec3& v, float t) { return fvec3(_mm_add_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator -(const fvec3& v, float t) { return fvec3(_mm_sub_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator *(const fvec3& v, float t) { return fvec3(_mm_mul_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator /(const fvec3& v, float t) { return fvec3(_mm_div_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator +(float t, const fvec3& v) { return fvec3(_mm_add_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator -(float t, const fvec3& v) { return fvec3(_mm_sub_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator *(float t, const fvec3& v) { return fvec3(_mm_mul_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator /(float t, const fvec3& v) { return fvec3(_mm_div_ps(v.R128, _mm_set_ps1(t))); }
  inline fvec3 operator +(const fvec3& u, const fvec3& v) { return fvec3(_mm_add_ps(u.R128, v.R128)); }
  inline fvec3 operator -(const fvec3& u, const fvec3& v) { return fvec3(_mm_sub_ps(u.R128, v.R128)); }
  inline fvec3 operator *(const fvec3& u, const fvec3& v) { return fvec3(_mm_mul_ps(u.R128, v.R128)); }
  inline fvec3 operator /(const fvec3& u, const fvec3& v) { return fvec3(_mm_div_ps(u.R128, v.R128)); }
#else
  inline vec3 operator + (const vec3& v, float t) { return vec3(v.x + t, v.y + t, v.z + t); }
  inline vec3 operator - (const vec3& v, float t) { return vec3(v.x - t, v.y - t, v.z - t); }
  inline vec3 operator * (const vec3& v, float t) { return vec3(v.x * t, v.y * t, v.z * t); }
  inline vec3 operator / (const vec3& v, float t) { return vec3(v.x / t, v.y / t, v.z / t); }
  inline vec3 operator + (float t, const vec3& v) { return v + t; }
  inline vec3 operator - (float t, const vec3& v) { return v - t; }
  inline vec3 operator * (float t, const vec3& v) { return v * t; }
  inline vec3 operator / (float t, const vec3& v) { return v / t; }
  inline vec3 operator + (const vec3& u, const vec3& v) { return vec3(u.x + v.x, u.y + v.y, u.z + v.z); }
  inline vec3 operator - (const vec3& u, const vec3& v) { return vec3(u.x - v.x, u.y - v.y, u.z - v.z); }
  inline vec3 operator * (const vec3& u, const vec3& v) { return vec3(u.x * v.x, u.y * v.y, u.z * v.z); }
  inline vec3 operator / (const vec3& u, const vec3& v) { return vec3(u.x / v.x, u.y / v.y, u.z / v.z); }
#endif
};
