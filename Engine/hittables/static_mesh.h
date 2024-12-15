#pragma once

#include <string>

#include "engine/hittable.h"

#include "engine/asset/soft_asset_ptr.h"
#include "assets/mesh.h"
#include "assets/material.h"

namespace reactphysics3d
{
  class BoxShape;
  class Collider;
}

namespace engine
{
  class ENGINE_API hstatic_mesh : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hstatic_mesh, hhittable_base)
    OBJECT_DECLARE_VISITOR

    CTOR_DEFAULT(hstatic_mesh)
    CTOR_COPY_DEFAULT(hstatic_mesh)
    CTOR_MOVE_DELETE(hstatic_mesh)
    virtual ~hstatic_mesh() override;
    
    virtual uint32_t get_hash() const override;
    virtual void load_resources() override;
    virtual void create_physics_state() override;
    virtual void destroy_physics_state() override;

    virtual void transform(const fvec3& in_origin, const fvec3& in_rotation, const fvec3& in_scale) override;

    void get_object_matrices(const XMFLOAT4X4& view_projection, fobject_data& out_data) const;

    // Persistent state
    fsoft_asset_ptr<astatic_mesh> mesh_asset_ptr;
    fsoft_asset_ptr<amaterial> material_asset_ptr;

    // Runtime members
    reactphysics3d::BoxShape* box_shape = nullptr;  // base: CollisionShape
    reactphysics3d::Collider* collider = nullptr;   
  };
}
