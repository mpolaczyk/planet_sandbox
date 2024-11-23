#include "hittables/scene.h"

#include "light.h"
#include "core/application.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "math/math.h"
#include "renderer/renderer_base.h"

namespace engine
{
  OBJECT_DEFINE(hscene, hhittable_base, Scene)
  OBJECT_DEFINE_SPAWN(hscene)
  OBJECT_DEFINE_VISITOR(hscene)

  hscene::~hscene()
  {
    destroy_scene_physics_state();
  }

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

  void hscene::create_scene_physics_state()
  {
    if(is_simulating_physics) return;

    for(hhittable_base* object : objects)
    {
      object->create_physics_state();
    }
    is_simulating_physics = true;
  }

  void hscene::update_scene_physics_state(float delta_time)
  {
    if(!is_simulating_physics) return;
    
    for(hhittable_base* object : objects)
    {
      object->update_physics_state(delta_time);
    }
  }

  void hscene::destroy_scene_physics_state()
  {
    if(!is_simulating_physics) return;

    for(hhittable_base* object : objects)
    {
      object->destroy_physics_state();
    }
    is_simulating_physics = false;
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
