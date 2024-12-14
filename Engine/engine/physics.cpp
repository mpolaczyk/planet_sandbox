
#include "engine/physics.h"

#include "core/application.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "math/aabb.h"
#include "math/math.h"

#include "reactphysics3d/engine/PhysicsCommon.h"
#include "reactphysics3d/engine/PhysicsWorld.h"
#include "reactphysics3d/collision/RaycastInfo.h"

namespace engine
{
  using namespace reactphysics3d;

  fphysics* fphysics::instance = nullptr;

  class raycast_callback : public RaycastCallback
  {
  public:
    float smallest_fraction = 1.0f;
    Body* closest_body = nullptr;
    Vector3 world_point;
    
  protected:
    virtual float notifyRaycastHit(const reactphysics3d::RaycastInfo& info) override
    {
      const float ray_terminate = 0.0f;
      const float ray_not_clipped_continue = 1.0f;
      const float ray_ignore_collider_continue = -1.0f;
      float ray_clip_continue = info.hitFraction;

      if(info.hitFraction < smallest_fraction)
      {
        closest_body = info.body;
        world_point = info.worldPoint;
        return ray_clip_continue;
      }
      return smallest_fraction;
    }
  };
  
  fphysics::~fphysics()
  {
    if(physics_world)
    {
      destroy_physics();
    }
    fphysics::instance = nullptr;
  }

  RigidBody* fphysics::create_rigid_body(const fvec3& origin, const fvec3& rotation, bool gravity_enabled, int32_t body_type)
  {
    RigidBody* rigid_body = fphysics::instance->physics_world->createRigidBody(Transform::identity());
    rigid_body->setTransform(fphysics::get_transform(origin, rotation));
    rigid_body->enableGravity(gravity_enabled);
    rigid_body->setType(static_cast<BodyType>(body_type));
    return rigid_body;
  }

  void fphysics::destroy_rigid_body(RigidBody* rigid_body)
  {
    if(!rigid_body) return;
    fphysics::instance->physics_world->destroyRigidBody(rigid_body);
  }

  void fphysics::reset_rigid_body(const fvec3& origin, const fvec3& rotation, RigidBody* out_rigid_body)
  {
    if(!out_rigid_body) return;
    out_rigid_body->setTransform(fphysics::get_transform(origin, rotation));
    out_rigid_body->setLinearVelocity(Vector3::zero());
    out_rigid_body->setAngularVelocity(Vector3::zero());
    out_rigid_body->resetForce();
    out_rigid_body->resetTorque();
  }

  void fphysics::set_rigid_body_transform(const fvec3& origin, const fvec3& rotation, RigidBody* out_rigid_body)
  {
    if(!out_rigid_body) return;
    out_rigid_body->setTransform(fphysics::get_transform(origin, rotation));
  }

  void fphysics::get_rigid_body_transform(RigidBody* rigid_body, fvec3& out_origin, fvec3& out_rotation)
  {
    if(!rigid_body) return;
    const Transform& transform = rigid_body->getTransform();

    const Vector3& position = transform.getPosition();
    out_origin.x = position.x;
    out_origin.y = position.y;
    out_origin.z = position.z;
    
    const Quaternion& quat = transform.getOrientation();
    const fvec3 rot_rad = fmath::quaternion_to_rpy(quat.x, quat.y, quat.z, quat.w);
    out_rotation.x = fmath::radians_to_degrees(rot_rad.x);
    out_rotation.y = fmath::radians_to_degrees(rot_rad.y);
    out_rotation.z = fmath::radians_to_degrees(rot_rad.z);
  }

  Transform fphysics::get_transform(const fvec3& origin, const fvec3& rotation)
  {
    const Vector3 position(origin.x, origin.y, origin.z);
    const Quaternion& orientation = Quaternion::fromEulerAngles(fmath::degrees_to_radians(rotation.x), fmath::degrees_to_radians(rotation.y), fmath::degrees_to_radians(rotation.z));
    return Transform(position, orientation);
  }

  BoxShape* fphysics::create_box_shape_collider(const fbounding_box& box, const fvec3& scale)
  {
    const reactphysics3d::Vector3 positive_extents(fabs(box.extents.x * scale.x), fabs(box.extents.y * scale.y), fabs(box.extents.z * scale.z));
    return fphysics::instance->physics_common->createBoxShape(positive_extents);
  }

  void fphysics::edit_box_shape_collider(BoxShape* box_shape, const fbounding_box& box, const fvec3& scale)
  {
    const reactphysics3d::Vector3 positive_extents(fabs(box.extents.x * scale.x), fabs(box.extents.y * scale.y), fabs(box.extents.z * scale.z));
    box_shape->setHalfExtents(positive_extents);
  }

  void fphysics::destroy_box_shape_collider(BoxShape* box_shape)
  {
    fphysics::instance->physics_common->destroyBoxShape(box_shape);
  }

  Collider* fphysics::attach_collider(RigidBody* rigid_body, CollisionShape* shape, const fbounding_box& box)
  {
    return rigid_body->addCollider(shape, Transform(Vector3(box.center.x, box.center.y, box.center.z), Quaternion::identity()));
  }

  void fphysics::detach_collider(RigidBody* rigid_body, Collider* collider)
  {
    rigid_body->removeCollider(collider);
  }

  fraycast_result fphysics::raycast_closest_body(const fray& ray, float ray_length)
  {
    const fvec3 ray_end = ray.at(ray_length);

    const Vector3 px_a(ray.origin.x, ray.origin.y, ray.origin.z);  // ?? camera.location
    const Vector3 px_b(ray_end.x, ray_end.y, ray_end.z);
    const Ray px_ray(px_a, px_b);

    raycast_callback px_callback;
    fphysics::instance->physics_world->raycast(px_ray, &px_callback);
    
    return fraycast_result(px_callback.closest_body, fvec3(px_callback.world_point.x, px_callback.world_point.y, px_callback.world_point.z));
  }

  void fphysics::create_physics(hscene* in_scene_root)
  {
    if(fphysics::instance)
    {
      LOG_ERROR("Physics state already initiated!")
      return;
    }
    if(!in_scene_root)
    {
      LOG_ERROR("Unable to initiate physics, invalid scene!")
      return;
    }
    fphysics::instance = this;
    scene_root = in_scene_root;

    // Physics common
    physics_common = std::make_shared<PhysicsCommon>();
    DefaultLogger* logger = physics_common->createDefaultLogger();
    uint log_level = static_cast<uint>(static_cast<uint>(Logger::Level::Warning) | static_cast<uint>(Logger::Level::Error));
    logger->addStreamDestination(std::cout, log_level, DefaultLogger::Format::Text);
    physics_common->setLogger(logger);

    // Physics world
    PhysicsWorld::WorldSettings settings;
    settings.isSleepingEnabled = false;
    settings.gravity = Vector3(0, -9.81f, 0);
    physics_world = physics_common->createPhysicsWorld(settings);

    // Scene
    scene_root->create_scene_physics_state();
  }

  void fphysics::update_physics(float delta_time)
  {
    if(wants_to_update && !update_enabled)
    {
      update_enabled = true;
      scene_root->save_pre_physics_scene_state();
    }
    else if(!wants_to_update && update_enabled)
    {
      update_enabled = false;
      scene_root->restore_pre_physics_scene_state();
    }
    if(update_enabled && delta_time != 0.0f)
    {
      physics_world->update(delta_time);
      scene_root->update_scene_physics_state();
    }
  }

  void fphysics::destroy_physics()
  {
    scene_root->destroy_scene_physics_state();
    physics_common->destroyPhysicsWorld(physics_world);
    physics_world = nullptr;
  }
}