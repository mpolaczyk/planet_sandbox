#include <sstream>

#include "hittables/hittables.h"

#include "core/application.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "engine/physics.h"
#include "math/math.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hhittable_base, oobject, Hittable)
  OBJECT_DEFINE_NOSPAWN(hhittable_base)
  OBJECT_DEFINE_VISITOR(hhittable_base)
  
  hhittable_base::~hhittable_base()
  {
    destroy_physics_state();
  }
  
  inline uint32_t hhittable_base::get_hash() const
  {
    return fhash::combine(oobject::get_hash(), fhash::get(origin), fhash::get(rotation), fhash::get(scale));
  }
  
  void hhittable_base::create_physics_state()
  {
    rigid_body = fphysics::create_rigid_body(origin, rotation, gravity_enabled, rigid_body_type);
    save_pre_physics_state();
  }

  void hhittable_base::save_pre_physics_state()
  {
    pre_physics_origin = origin;
    pre_physics_rotation = rotation;
  }

  void hhittable_base::restore_pre_physics_state()
  {
    if(!rigid_body) return;
    origin = pre_physics_origin;
    rotation = pre_physics_rotation;
    fphysics::reset_rigid_body(origin, rotation, rigid_body);
  }
  
  void hhittable_base::update_physics_state()
  {
    fphysics::get_rigid_body_transform(rigid_body, origin, rotation);
  }

  void hhittable_base::destroy_physics_state()
  {
    if(!rigid_body) return;
    origin = pre_physics_origin;
    rotation = pre_physics_rotation;
    fphysics::destroy_rigid_body(rigid_body);
    rigid_body = nullptr;
  }
}
