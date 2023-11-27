#pragma once

#include <string>

#include "hittables.h"
#include "object/object_types.h"

#include "math/vec3.h"
#include "math/ray.h"
#include "math/hit.h"
#include "math/aabb.h"
#include "asset/soft_asset_ptr.h"
#include "assets/mesh.h"

namespace engine
{
  class ENGINE_API static_mesh : public hittable
  {
  public:
    OBJECT_DECLARE(static_mesh, object)

    virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
    virtual bool get_bounding_box(aabb& out_box) const override;
    virtual std::string get_name() const override;
    virtual vec3 get_origin() const override { return origin; };
    virtual vec3 get_extent() const override { return vec3(extent); };
    virtual void set_origin(const vec3& value) override { origin = value; };
    virtual void set_extent(float value) override { extent = value; };

    virtual uint32_t get_hash() const override;
    virtual static_mesh* clone() const override;
    virtual void load_resources() override;
    virtual void pre_render() override;

    // Persistent state
    vec3 origin = vec3(0, 0, 0);
    vec3 scale = vec3(1, 1, 1);
    vec3 rotation = vec3(0, 0, 0);  // degrees
    soft_asset_ptr<static_mesh_asset> mesh_asset_ptr;

    // Runtime state
    static_mesh_asset* runtime_asset_ptr;  // Vertices translated to the world coordinates

    mutable float extent = 0.0f;
  };
}