#pragma once

#include <string>

#include "hittables.h"


#include "math/vec3.h"
#include "math/ray.h"
#include "math/hit.h"
#include "math/aabb.h"

namespace engine
{
  class sphere;

  class ENGINE_API sphere : public hittable
  {
  public:
    OBJECT_DECLARE(sphere, hittable)
    OBJECT_DECLARE_VISITOR
    
    virtual bool hit(const ray& in_ray, float t_max, hit_record& out_hit) const override;
    virtual bool get_bounding_box(aabb& out_box) const override;
    virtual vec3 get_origin() const override { return origin; };
    virtual vec3 get_extent() const override { return vec3(radius); };
    virtual void set_origin(const vec3& value) override { origin = value; };
    virtual void set_extent(float value) override { radius = value; };

    virtual uint32_t get_hash() const override;
    virtual sphere* clone() const override;

    // Persistent members
    vec3 origin = vec3(0, 0, 0);
    float radius = 0.0f;
  };
}