#pragma once

#include <string>

#include "hittables.h"


#include "math/vec3.h"
#include "math/ray.h"
#include "math/hit.h"
#include "math/aabb.h"

namespace engine
{
  class hsphere;

  class ENGINE_API hsphere : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hsphere, hhittable_base)
    OBJECT_DECLARE_VISITOR
    
    virtual bool hit(const fray& in_ray, float t_max, fhit_record& out_hit) const override;
    virtual bool get_bounding_box(faabb& out_box) const override;
    virtual fvec3 get_origin() const override { return origin; };
    virtual fvec3 get_extent() const override { return fvec3(radius); };
    virtual void set_origin(const fvec3& value) override { origin = value; };
    virtual void set_extent(float value) override { radius = value; };

    virtual uint32_t get_hash() const override;
    virtual hsphere* clone() const override;

    // Persistent members
    fvec3 origin = fvec3(0, 0, 0);
    float radius = 0.0f;
  };
}