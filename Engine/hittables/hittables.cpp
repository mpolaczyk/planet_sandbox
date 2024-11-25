#include <sstream>

#include "hittables/hittables.h"

#include "core/application.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "math/math.h"
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
    
    // Needs to happen after persistent state is loaded
    const Vector3 position(origin.x, origin.y, origin.z);
    const Quaternion& orientation = Quaternion::fromEulerAngles(fmath::degrees_to_radians(rotation.x), fmath::degrees_to_radians(rotation.y), fmath::degrees_to_radians(rotation.z));
    const Transform transform(position, orientation);
    rigid_body = fapplication::get_instance()->physics_world->createRigidBody(transform);
    rigid_body->enableGravity(gravity_enabled);
    rigid_body->setType(static_cast<BodyType>(rigid_body_type));

    set_physics_state();
  }

  void hhittable_base::set_physics_state()
  {
    using namespace reactphysics3d;
    
    pre_physics_origin = origin;
    pre_physics_rotation = rotation;
  }

  void hhittable_base::reset_physics_state()
  {
    if(!rigid_body) return;
    using namespace reactphysics3d;

    origin = pre_physics_origin;
    rotation = pre_physics_rotation;

    const Vector3 velocity(0.0f, 0.0f, 0.0f);
    const Vector3 position(origin.x, origin.y, origin.z);
    const Quaternion& orientation = Quaternion::fromEulerAngles(fmath::degrees_to_radians(rotation.x), fmath::degrees_to_radians(rotation.y), fmath::degrees_to_radians(rotation.z));
    const Transform transform(position, orientation);
    rigid_body->setTransform(transform);
    rigid_body->setLinearVelocity(velocity);
    rigid_body->resetForce();
    rigid_body->resetTorque();
  }
  
  void hhittable_base::update_physics_state(float delta_time)
  {
    using namespace reactphysics3d;

    if(!rigid_body) return;
    if(delta_time == 0.0f) return;
    
    const Transform& transform = rigid_body->getTransform();

    const Vector3& position = transform.getPosition();
    origin.x = position.x;
    origin.y = position.y;
    origin.z = position.z;
    
    const Quaternion& quat = transform.getOrientation();
    const fvec3 rot_rad = fmath::quaternion_to_rpy(quat.x, quat.y, quat.z, quat.w);
    rotation.x = fmath::radians_to_degrees(rot_rad.x);
    rotation.y = fmath::radians_to_degrees(rot_rad.y);
    rotation.z = fmath::radians_to_degrees(rot_rad.z);
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
