#pragma once

#include "core/core.h"
#include "math/vec3.h"
#include "object/object.h"

namespace reactphysics3d
{
  class RigidBody;
}

namespace engine
{
  class ENGINE_API hhittable_base : public oobject
  {
  public:
    OBJECT_DECLARE(hhittable_base, oobject)
    OBJECT_DECLARE_VISITOR

    CTOR_DEFAULT(hhittable_base)
    CTOR_COPY_DEFAULT(hhittable_base)
    CTOR_MOVE_DELETE(hhittable_base)
    virtual ~hhittable_base() override;

    virtual uint32_t get_hash() const override;
    
    virtual void load_resources()
    {
    };

    virtual void create_physics_state();
    void save_pre_physics_state();
    void restore_pre_physics_state();
    void update_physics_state();
    virtual void destroy_physics_state();
    
    virtual void transform(const fvec3& in_origin, const fvec3& in_rotation, const fvec3& in_scale);
    
    // Persistent members
    fvec3 origin = fvec3(0.0f, 0.0f, 0.0f);
    fvec3 scale = fvec3(1.0f, 1.0f, 1.0f);
    fvec3 rotation = fvec3(0.0f, 0.0f, 0.0f); // degrees
    bool gravity_enabled = false;
    int32_t rigid_body_type = 0; // maps to reactphysics3d::BodyType
    
    // Runtime members
    reactphysics3d::RigidBody* rigid_body;  // ownership
    fvec3 pre_physics_origin = fvec3(0.0f, 0.0f, 0.0f);
    fvec3 pre_physics_rotation = fvec3(0.0f, 0.0f, 0.0f); // degrees
  };
}
