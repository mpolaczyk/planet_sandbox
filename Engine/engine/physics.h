#pragma once

#include <memory>

#include "core/core.h"
#include "math/vec3.h"

namespace reactphysics3d
{
  class Transform;
  class PhysicsCommon;
  class PhysicsWorld;
  class Body;
  class RigidBody;
  class BoxShape;
  class Collider;
  class CollisionShape;
}

namespace engine
{
  class hscene;
  struct fray;
  struct fbounding_box;
  
  using namespace reactphysics3d;

  struct ENGINE_API fraycast_result
  {
    CTOR_DEFAULT(fraycast_result)
    fraycast_result(Body* in_body, const fvec3&& in_world_point)
      : body(in_body), world_point(std::move(in_world_point))
    {}
    
    Body* body;
    fvec3 world_point;
  };
  
  struct ENGINE_API fphysics final
  {
  public:
    // Physics tools for other classes
    static RigidBody* create_rigid_body(const fvec3& origin, const fvec3& rotation, bool gravity_enabled, int32_t body_type);
    static void destroy_rigid_body(RigidBody* rigid_body);
    static void reset_rigid_body(const fvec3& origin, const fvec3& rotation, RigidBody* out_rigid_body);
    static void set_rigid_body_transform(const fvec3& origin, const fvec3& rotation, RigidBody* out_rigid_body);
    static void get_rigid_body_transform(RigidBody* rigid_body, fvec3& out_origin, fvec3& out_rotation);
    static Transform get_transform(const fvec3& origin, const fvec3& rotation);
    static BoxShape* create_box_shape_collider(const fbounding_box& box, const fvec3& scale);
    static void destroy_box_shape_collider(BoxShape* box_shape);
    static Collider* attach_collider(RigidBody* rigid_body, CollisionShape* shape, const fbounding_box& box);
    static void detach_collider(RigidBody* rigid_body, Collider* collider);
    static fraycast_result raycast_closest_body(const fray& ray, float ray_length);

    
  private:
    static fphysics* instance;

  public:
    CTOR_DEFAULT(fphysics)
    CTOR_MOVE_COPY_DELETE(fphysics)
    ~fphysics();

    // Main state management
    void create_physics(hscene* in_scene_root);
    void update_physics(float delta_time);
    void destroy_physics();

    // Simulation state
    bool is_simulating() const { return update_enabled; }
    void start_simulating() { wants_to_update = true; }
    void stop_simulating() { wants_to_update = false; }
    void toggle_simulating() { wants_to_update = !wants_to_update; }
    
    // Third party library resources, ownership
    std::shared_ptr<PhysicsCommon> physics_common;
    PhysicsWorld* physics_world = nullptr;
    //std::shared_ptr<PhysicsWorld> physics_world;  // TODO some nonsense compilation issues

  private:
    hscene* scene_root = nullptr; // no ownership

    // State machine
    bool wants_to_update = false;
    bool update_enabled = false;
  };
}