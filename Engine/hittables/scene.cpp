

#include "hittables/scene.h"

#include "light.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"
#include "hittables/static_mesh.h"
#include "hittables/sphere.h"
#include "object/object_visitor.h"
#include "math/math.h"

namespace engine
{
  OBJECT_DEFINE(hscene, hhittable_base, Scene)
  OBJECT_DEFINE_SPAWN(hscene)
  OBJECT_DEFINE_VISITOR(hscene)

  inline uint32_t hscene::get_hash() const
  {
    uint32_t a = 0;
    for (const hhittable_base* obj : objects)
    {
      a = fhash::combine(a, obj->get_hash());
    }
    return a;
  }
  
  void hscene::load_resources()
  {
    LOG_TRACE("Scene: load resources");

    for (hhittable_base* object : objects)
    {
      assert(object != nullptr);
      object->load_resources();
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
    for (const hhittable_base* obj : objects)
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

 

  //hscene* hscene::clone() const
  //{
  //  throw std::runtime_error("Attempt to clone the scene!"); // Is this still needed?
  //  
  //  hscene* new_scene = REG.copy_shallow<hscene>(this);
  //  // Deep copy
  //  for (hhittable_base* obj : objects)
  //  {
  //    const oclass_object* class_o = obj->get_class();
  //    hhittable_base* new_obj = nullptr;
  //    if (class_o == oclass_object::get_class_static())
  //    {
  //      new_obj = REG.copy_shallow<hscene>(static_cast<hscene*>(obj));
  //    }
  //    else if (class_o == hsphere::get_class_static())
  //    {
  //      new_obj = REG.copy_shallow<hsphere>(static_cast<hsphere*>(obj));
  //    }
  //    else if (class_o == hstatic_mesh::get_class_static())
  //    {
  //      new_obj = REG.copy_shallow<hstatic_mesh>(static_cast<hstatic_mesh*>(obj));
  //    }
  //    else if (class_o == hlight::get_class_static())
  //    {
  //      new_obj = REG.copy_shallow<hlight>(static_cast<hlight*>(obj));
  //    }
  //    else
  //    {
  //      LOG_ERROR("Unable to clone a hittable of type: {0}", obj->get_class()->get_class_name());
  //      return nullptr;
  //    }
  //    
  //    new_scene->objects.push_back(new_obj);
  //  }
  //  return new_scene;
  //}
}