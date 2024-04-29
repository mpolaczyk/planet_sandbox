#pragma once

#include <float.h>

#include "core/core.h"

#include "math/vec3.h"
#include "math/ray.h"
#include "math/hit.h"
#include "vertex_data.h"

namespace DirectX
{
  struct XMFLOAT4;
}

namespace engine
{
  class ENGINE_API fmath
  {
  public:
    static constexpr float infinity = FLT_MAX;
    static constexpr float pi = 3.1415926535897932385f;
    static constexpr float small_number = 0.0001f;
    static constexpr float very_small_number = 0.00000001f;
    static constexpr float epsilon = FLT_EPSILON;
    static constexpr float t_min = 0.01f;

    // FLOAT
    static float reflectance(float cosine, float ref_idx);
    static float sign(float value);
    static float degrees_to_radians(float degrees);
    static bool is_almost_zero(float value);
    static bool is_almost_equal(float a, float b);
    static float inv_sqrt(float x);
    static float min1(float a, float b);
    static float max1(float a, float b);
    static float clamp(float f, float a, float b);
    static float smoothstep(float a, float b, float x);
    static float lerp_float(float a, float b, float f);

    // VEC3
    static fvec3 reflect(const fvec3& v, const fvec3& n);
    static fvec3 refract(const fvec3& uv, const fvec3& n, float etai_over_etat);
    static bool flip_normal_if_front_face(const fvec3& in_ray_direction, const fvec3& in_outward_normal, fvec3& out_normal);
    static fvec3 lerp_vec3(const fvec3& a, const fvec3& b, float f);
    static fvec3 clamp_vec3(float a, float b, const fvec3& f);
    static bool ray_triangle(const fray& ray, float t_max, const ftriangle_face* in_triangle, fhit_record& out_hit, bool drop_backface = false);
    static bool is_near_zero(const fvec3& value);
    static bool is_zero(const fvec3& value);
    static float dot(const fvec3& u, const fvec3& v);
    static fvec3 cross(const fvec3& u, const fvec3& v);
    static fvec3 normalize(const fvec3& v);
    static float length(const fvec3& v);
    static float length_squared(const fvec3& v);
    static fvec3 rotate_yaw(const fvec3& u, float yaw);
    static fvec3 rotate_pitch(const fvec3& u, float pitch);
    static fvec3 rotate_roll(const fvec3& u, float roll);
    static fvec3 rpy_to_direction(const fvec3& rpy);
    static fvec3 min3(const fvec3& a, const fvec3& b);
    static fvec3 max3(const fvec3& a, const fvec3& b);
    static void get_sphere_uv(const fvec3& p, float& out_u, float& out_v);
    static fvec3 to_vec3(const DirectX::XMFLOAT4& a);
    static DirectX::XMFLOAT4 to_xmfloat4(const fvec3& a);
  };
}