#pragma once

#include <float.h>

#include "core/core.h"

#include "math/vec3.h"
#include "math/ray.h"
#include "math/hit.h"
#include "math/triangle_face.h"

namespace engine
{
  class ENGINE_API math
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
    static vec3 reflect(const vec3& v, const vec3& n);
    static vec3 refract(const vec3& uv, const vec3& n, float etai_over_etat);
    static bool flip_normal_if_front_face(const vec3& in_ray_direction, const vec3& in_outward_normal, vec3& out_normal);
    static vec3 lerp_vec3(const vec3& a, const vec3& b, float f);
    static vec3 clamp_vec3(float a, float b, const vec3& f);
    static bool ray_triangle(const ray& ray, float t_max, const triangle_face* in_triangle, hit_record& out_hit, bool drop_backface = false);
    static bool is_near_zero(const vec3& value);
    static bool is_zero(const vec3& value);
    static float dot(const vec3& u, const vec3& v);
    static vec3 cross(const vec3& u, const vec3& v);
    static vec3 normalize(const vec3& v);
    static float length(const vec3& v);
    static float length_squared(const vec3& v);
    static vec3 rotate_yaw(const vec3& u, float yaw);
    static vec3 rotate_pitch(const vec3& u, float pitch);
    static vec3 rotate_roll(const vec3& u, float roll);
    static vec3 min3(const vec3& a, const vec3& b);
    static vec3 max3(const vec3& a, const vec3& b);
    static void get_sphere_uv(const vec3& p, float& out_u, float& out_v);
  };
}