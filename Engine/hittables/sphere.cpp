
#include <sstream>

#include "hittables/sphere.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "math/math.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hsphere, hhittable_base, Sphere)
  OBJECT_DEFINE_SPAWN(hsphere)
  OBJECT_DEFINE_VISITOR(hsphere)
  
  bool hsphere::hit(const fray& in_ray, float t_max, fhit_record& out_hit) const
  {
    fvec3 oc = in_ray.origin - origin;
    float a = fmath::length_squared(in_ray.direction);
    float half_b = fmath::dot(oc, in_ray.direction);
    float c = fmath::length_squared(oc) - radius * radius;

    float delta = half_b * half_b - a * c;
    if (delta < 0.0f)
    {
      return false;
    }

    // Find the nearest root that lies in the acceptable range.
    float sqrtd = sqrt(delta);
    float root = (-half_b - sqrtd) / a;
    if (root < fmath::t_min || t_max < root)
    {
      root = (-half_b + sqrtd) / a;
      if (root < fmath::t_min || t_max < root)
      {
        return false;
      }
    }

    out_hit.t = root;
    out_hit.p = in_ray.at(out_hit.t);
    out_hit.material_ptr = material_asset_ptr.get();

    // Normal always against the ray
    fvec3 outward_normal = (out_hit.p - origin) / radius;
    out_hit.front_face = fmath::flip_normal_if_front_face(in_ray.direction, outward_normal, out_hit.normal);
    fmath::get_sphere_uv(outward_normal, out_hit.u, out_hit.v);
    return true;
  }

  bool hsphere::get_bounding_box(faabb& out_box) const
  {
    out_box = faabb(origin - radius, origin + radius);
    return true;
  }

  inline uint32_t hsphere::get_hash() const
  {
    return fhash::combine(hhittable_base::get_hash(), fhash::get(origin), fhash::get(radius));
  }


  hsphere* hsphere::clone() const
  {
    return REG.copy_shallow<hsphere>(this);
  }

}