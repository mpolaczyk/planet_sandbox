#pragma once

#include <array>
#include <assert.h>

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

  class ENGINE_API hittable : public object
  {
  public:
    OBJECT_DECLARE(hittable, object)

    virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const = 0;
    virtual bool get_bounding_box(aabb& out_box) const = 0;
    virtual vec3 get_origin() const = 0;
    virtual vec3 get_extent() const = 0;
    virtual void set_origin(const vec3& value) = 0;
    virtual void set_extent(float value) = 0;
    // Deprecated begin
    virtual float get_area() const { assert(false); return 0.0f; };
    virtual float get_pdf_value(const vec3& origin, const vec3& v) const { assert(false); return 0.0f; };
    virtual vec3 get_pdf_direction(const vec3& look_from) const { assert(false); return vec3(0, 0, 0); };
    virtual vec3 get_random_point() const { assert(false); return vec3(0, 0, 0); };
    // Deprecated end

    virtual uint32_t get_hash() const;
    virtual hittable* clone() const = 0;
    virtual void load_resources();
    virtual void pre_render() {};

    // Persistent members
    soft_asset_ptr<material_asset> material_asset_ptr;    // FIX This does not have to be here, move to mesh and sphere?

    // Runtime members
    aabb bounding_box;
  };

  



}