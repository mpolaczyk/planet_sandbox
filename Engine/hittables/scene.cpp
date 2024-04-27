
#include "hittables/scene.h"
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
    delete objects[object_id];
    objects.erase(objects.begin() + object_id);
  }

  void hscene::build_boxes()
  {
    LOG_TRACE("Scene: build boxes");

    assert(objects.size() > 0);
    // World collisions update
    for (hhittable_base* object : objects)
    {
      assert(object != nullptr);
      object->get_bounding_box(object->bounding_box);
    }
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

  void hscene::query_lights()
  {
    LOG_TRACE("Scene: query lights");

    lights_num = 0;
    for (hhittable_base* object : objects)
    {
      const amaterial* mat = object->material_asset_ptr.get();
      if (mat != nullptr && mat->is_light)
      {
        lights[lights_num] = object;
        lights_num++;
        assert(lights_num < MAX_LIGHTS);
      }
    }
    if (lights_num == 0)
    {
      LOG_WARN("No lights detected.");
    }
  }

  hhittable_base* hscene::get_random_light()
  {
    assert(lights_num < MAX_LIGHTS);
    // Get next light millions of times gives the same distribution as get true random light, but is 5 times cheaper
    static int32_t last_light = 0;
    last_light = (last_light + 1) % lights_num;
    hhittable_base* light = lights[last_light];
    assert(light != nullptr);
    return light;
  }

  bool hscene::get_bounding_box(faabb& out_box) const
  {
    if (objects.empty()) return false;

    faabb temp_box;
    bool first_box = true;

    for (const hhittable_base* object : objects)
    {
      if (!object->get_bounding_box(temp_box)) return false;
      out_box = first_box ? temp_box : faabb::merge(out_box, temp_box);
      first_box = false;
    }

    return true;
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