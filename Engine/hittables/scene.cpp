
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
  
  void hscene::add(hhittable_base* object)
  {
    objects.push_back(object);
  }

  void hscene::remove(int object_id)
  { 
    objects[object_id]->destroy();
    objects.erase(objects.begin() + object_id);
  }

  void hscene::update_materials()
  {
    LOG_TRACE("Scene: update materials");

    assert(objects.size() > 0);

    // Trigger resource loading for materials.
    // Soft ptr name change may invalidate it.
    for (hhittable_base* obj : objects)
    {
      obj->material_asset_ptr.get();
    }
  }

  std::vector<const hlight*> hscene::query_lights() const
  {
    std::vector<const hlight*> lights;
    for (const hhittable_base* obj : objects)
    {
      if(obj->get_class() == hlight::get_class_static())
      {
        lights.push_back(static_cast<const hlight*>(obj));
      }
    }
    return lights;
  }

  inline uint32_t hscene::get_hash() const
  {
    uint32_t a = 0;
    for (const hhittable_base* obj : objects)
    {
      a = fhash::combine(a, obj->get_hash());
    }
    return a;
  }

  hscene* hscene::clone() const
  {
    hscene* new_scene = REG.copy_shallow<hscene>(this);
    // Deep copy
    for (hhittable_base* obj : objects)
    {
      const oclass_object* class_o = obj->get_class();
      hhittable_base* new_obj = nullptr;
      if (class_o == oclass_object::get_class_static())
      {
        new_obj = REG.copy_shallow<hscene>(static_cast<hscene*>(obj));
      }
      else if (class_o == hsphere::get_class_static())
      {
        new_obj = REG.copy_shallow<hsphere>(static_cast<hsphere*>(obj));
      }
      else if (class_o == hstatic_mesh::get_class_static())
      {
        new_obj = REG.copy_shallow<hstatic_mesh>(static_cast<hstatic_mesh*>(obj));
      }
      else
      {
        LOG_ERROR("Unable to clone a hittable of type: {0}", obj->get_class()->get_class_name());
        return nullptr;
      }
      
      new_scene->objects.push_back(new_obj);
    }
    return new_scene;
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
}