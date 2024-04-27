#pragma once

#include <string>

#include "hittables.h"


#include "math/vec3.h"
#include "math/ray.h"
#include "math/hit.h"
#include "math/aabb.h"
#include "asset/soft_asset_ptr.h"
#include "assets/mesh.h"

namespace engine
{
  class ENGINE_API hstatic_mesh : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hstatic_mesh, hhittable_base)
    OBJECT_DECLARE_VISITOR
    
    virtual bool get_bounding_box(faabb& out_box) const override;
    virtual fvec3 get_origin() const override { return origin; };
    virtual fvec3 get_extent() const override { return fvec3(extent); };
    virtual void set_origin(const fvec3& value) override { origin = value; };
    virtual void set_extent(float value) override { extent = value; };

    virtual uint32_t get_hash() const override;
    virtual hstatic_mesh* clone() const override;
    virtual void load_resources() override;

    // Persistent state
    fvec3 origin = fvec3(0, 0, 0);
    fvec3 scale = fvec3(1, 1, 1);
    fvec3 rotation = fvec3(0, 0, 0);  // degrees
    fsoft_asset_ptr<astatic_mesh> mesh_asset_ptr;

    // Runtime state, only for CPU renderer
    astatic_mesh* runtime_asset_ptr;  // Vertices translated to the world coordinates

    mutable float extent = 0.0f;
  };
}