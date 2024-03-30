#pragma once

#include <array>
#include <cassert>

#include "core/core.h"

#include "math/vec3.h"
#include "math/aabb.h"
#include "math/hit.h"
#include "math/ray.h"

#include "object/object.h"

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "assets/mesh.h"


namespace engine
{
  constexpr int32_t MAX_LIGHTS = 50;

  class ENGINE_API hhittable_base : public oobject
  {
  public:
    OBJECT_DECLARE(hhittable_base, oobject)
    OBJECT_DECLARE_VISITOR
    
    virtual bool hit(const fray& in_ray, float t_max, fhit_record& out_hit) const = 0;
    virtual bool get_bounding_box(faabb& out_box) const = 0;
    virtual fvec3 get_origin() const = 0;
    virtual fvec3 get_extent() const = 0;
    virtual void set_origin(const fvec3& value) = 0;
    virtual void set_extent(float value) = 0;
    // Deprecated begin
    virtual float get_area() const { assert(false); return 0.0f; };
    virtual float get_pdf_value(const fvec3& origin, const fvec3& v) const { assert(false); return 0.0f; };
    virtual fvec3 get_pdf_direction(const fvec3& look_from) const { assert(false); return fvec3(0, 0, 0); };
    virtual fvec3 get_random_point() const { assert(false); return fvec3(0, 0, 0); };
    // Deprecated end

    virtual uint32_t get_hash() const;
    virtual hhittable_base* clone() const = 0;
    virtual void load_resources();
    virtual void pre_render() {};

    // Persistent members
    fsoft_asset_ptr<amaterial> material_asset_ptr;

    // Runtime members
    faabb bounding_box;
  };

  



}