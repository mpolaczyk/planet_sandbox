#include "hittables/scene.h"

#include "light.h"
#include "core/application.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"
#include "hittables/static_mesh.h"
#include "hittables/sphere.h"
#include "object/object_visitor.h"
#include "math/math.h"

#include "reactphysics3d/engine/PhysicsCommon.h"
#include "reactphysics3d/engine/PhysicsWorld.h"

namespace engine
{
  OBJECT_DEFINE(hscene, hhittable_base, Scene)
  OBJECT_DEFINE_SPAWN(hscene)
  OBJECT_DEFINE_VISITOR(hscene)

  inline uint32_t hscene::get_hash() const
  {
    uint32_t a = 0;
    for(const hhittable_base* obj : objects)
    {
      a = fhash::combine(a, obj->get_hash());
    }
    return a;
  }

  void hscene::load_resources()
  {
    LOG_TRACE("Scene: loading resources");
    
    for(hhittable_base* object : objects)
    {
      assert(object != nullptr);
      object->load_resources();
    }
  }

  void hscene::create_physics_state()
  {
    LOG_TRACE("Scene: creating physics scene");

    using namespace reactphysics3d;
    physics_world = fapplication::get_instance()->physics_common->createPhysicsWorld();
    for(hhittable_base* object : objects)
    {
      Vector3 position(object->origin.x, object->origin.y, object->origin.z);
      Quaternion orientation = Quaternion::fromEulerAngles(object->rotation.x, object->rotation.y, object->rotation.z);
      Transform transform(position, orientation);
      RigidBody* rb = physics_world->createRigidBody(transform);
      object->rigid_body.reset(rb);
    }
  }

  void hscene::add(hhittable_base* object)
  {
    objects.push_back(object);
  }

  void hscene::remove(int object_id)
  {
    objects[object_id]->destroy();
    objects.erase(objects.begin() + object_id);
  }

  std::vector<const hlight*> hscene::query_lights() const
  {
    std::vector<const hlight*> lights;
    for(const hhittable_base* obj : objects)
    {
      if(obj->get_class() == hlight::get_class_static())
      {
        const hlight* light = static_cast<const hlight*>(obj);
        if(light->properties.enabled)
        {
          lights.push_back(light);
        }
      }
    }
    return lights;
  }
}
