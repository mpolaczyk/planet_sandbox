#include <sstream>

#include "hittables/hittables.h"

#include "core/application.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

#include "reactphysics3d/engine/PhysicsCommon.h"

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
    using namespace reactphysics3d;

    // State before physics state is created (use to reset the simulation)
    pre_physics_origin = origin;
    pre_physics_rotation = rotation;
    
    // Needs to happen after persistent state is loaded
    const Vector3 position(origin.x, origin.y, origin.z);
    const Quaternion& orientation = Quaternion::fromEulerAngles(rotation.x, rotation.y, rotation.z);
    const Transform transform(position, orientation);
    rigid_body = fapplication::get_instance()->physics_world->createRigidBody(transform);
    rigid_body->enableGravity(gravity_enabled);
    rigid_body->setType(static_cast<BodyType>(rigid_body_type));
  }

  void hhittable_base::update_physics_state(float delta_time)
  {
    using namespace reactphysics3d;

    if(!rigid_body) return;
    if(delta_time == 0.0f) return;
    
    const Transform& transform = rigid_body->getTransform();
    const Vector3& position = transform.getPosition();
    //const Quaternion& rotation = transform.getOrientation();  // TODO quaternion to euler angles
    origin.x = position.x;
    origin.y = position.y;
    origin.z = position.z;
  }

  void hhittable_base::destroy_physics_state()
  {
    if(!rigid_body) return;

    origin = pre_physics_origin;
    rotation = pre_physics_rotation;

    fapplication::get_instance()->physics_world->destroyRigidBody(rigid_body);
    rigid_body = nullptr;
  }
}
