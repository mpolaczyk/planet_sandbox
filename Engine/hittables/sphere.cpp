
#include <sstream>

#include "hittables/sphere.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "math/math.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(sphere, hittable, Sphere)
  OBJECT_DEFINE_SPAWN(sphere)

  bool sphere::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
  {
    vec3 oc = in_ray.origin - origin;
    float a = math::length_squared(in_ray.direction);
    float half_b = math::dot(oc, in_ray.direction);
    float c = math::length_squared(oc) - radius * radius;

    float delta = half_b * half_b - a * c;
    if (delta < 0.0f)
    {
      return false;
    }

    // Find the nearest root that lies in the acceptable range.
    float sqrtd = sqrt(delta);
    float root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root)
    {
      root = (-half_b + sqrtd) / a;
      if (root < t_min || t_max < root)
      {
        return false;
      }
    }

    out_hit.t = root;
    out_hit.p = in_ray.at(out_hit.t);
    out_hit.material_ptr = material_asset_ptr.get();

    // Normal always against the ray
    vec3 outward_normal = (out_hit.p - origin) / radius;
    out_hit.front_face = math::flip_normal_if_front_face(in_ray.direction, outward_normal, out_hit.normal);
    math::get_sphere_uv(outward_normal, out_hit.u, out_hit.v);
    return true;
  }

  bool sphere::get_bounding_box(aabb& out_box) const
  {
    out_box = aabb(origin - radius, origin + radius);
    return true;
  }

  inline uint32_t sphere::get_hash() const
  {
    return hash::combine(hittable::get_hash(), hash::get(origin), hash::get(radius));
  }


  sphere* sphere::clone() const
  {
    return REG.copy_shallow<sphere>(this);
  }

}