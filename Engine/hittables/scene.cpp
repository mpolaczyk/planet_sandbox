
#include "hittables/scene.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"
#include "static_mesh.h"
#include "sphere.h"

namespace engine
{
  OBJECT_DEFINE(scene, hittable, Scene)
  OBJECT_DEFINE_SPAWN(scene)

  void scene::add(hittable* object)
  {
    objects.push_back(object);
  }

  void scene::remove(int object_id)
  {
    delete objects[object_id];
    objects.erase(objects.begin() + object_id);
  }

  void scene::build_boxes()
  {
    LOG_TRACE("Scene: build boxes");

    assert(objects.size() > 0);
    // World collisions update
    for (hittable* object : objects)
    {
      assert(object != nullptr);
      object->get_bounding_box(object->bounding_box);
    }
  }

  void scene::update_materials()
  {
    LOG_TRACE("Scene: update materials");

    assert(objects.size() > 0);

    // Trigger resource loading for materials.
    // Soft ptr name change may invalidate it.
    for (hittable* obj : objects)
    {
      obj->material_asset_ptr.get();
    }
  }

  void scene::query_lights()
  {
    LOG_TRACE("Scene: query lights");

    lights_num = 0;
    for (hittable* object : objects)
    {
      const material_asset* mat = object->material_asset_ptr.get();
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

  hittable* scene::get_random_light()
  {
    assert(lights_num < MAX_LIGHTS);
    // Get next light millions of times gives the same distribution as get true random light, but is 5 times cheaper
    static int32_t last_light = 0;
    last_light = (last_light + 1) % lights_num;
    hittable* light = lights[last_light];
    assert(light != nullptr);
    return light;
  }



  bool scene::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
  {
    hit_record temp_rec;
    bool hit_anything = false;
    float closest_so_far = t_max;

    for (hittable* object : objects)
    {
#if USE_TLAS
      if (!object->bounding_box.hit(in_ray, t_min, t_max))
      {
        continue;
      }
#endif USE_TLAS
      stats::inc_ray_object_intersection();
      if (object->hit(in_ray, t_min, closest_so_far, temp_rec))
      {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        out_hit = temp_rec;
        out_hit.object = object;
      }
    }

    return hit_anything;
  }

  bool scene::get_bounding_box(aabb& out_box) const
  {
    if (objects.empty()) return false;

    aabb temp_box;
    bool first_box = true;

    for (const hittable* object : objects)
    {
      if (!object->get_bounding_box(temp_box)) return false;
      out_box = first_box ? temp_box : aabb::merge(out_box, temp_box);
      first_box = false;
    }

    return true;
  }

  inline uint32_t scene::get_hash() const
  {
    uint32_t a = 0;
    for (const hittable* obj : objects)
    {
      a = hash::combine(a, obj->get_hash());
    }
    return a;
  }

  scene* scene::clone() const
  {
    scene* new_scene = get_object_registry()->copy_shallow<scene>(this);
    // Deep copy
    for (hittable* obj : objects)
    {
      const class_object* class_o = obj->get_class();
      hittable* new_obj = nullptr;
      if (class_o == class_object::get_class_static())
      {
        new_obj = get_object_registry()->copy_shallow<scene>(static_cast<scene*>(obj));
      }
      else if (class_o == sphere::get_class_static())
      {
        new_obj = get_object_registry()->copy_shallow<sphere>(static_cast<sphere*>(obj));
      }
      else if (class_o == static_mesh::get_class_static())
      {
        new_obj = get_object_registry()->copy_shallow<static_mesh>(static_cast<static_mesh*>(obj));
      }
      else
      {
        LOG_ERROR("Unable to clone a hittable of type: {0}", obj->get_class()->class_name.c_str());
        return nullptr;
      }
      
      new_scene->objects.push_back(new_obj);
    }
    return new_scene;
  }


  void scene::load_resources()
  {
    LOG_TRACE("Scene: load resources");

    for (hittable* object : objects)
    {
      assert(object != nullptr);
      object->load_resources();
    }
  }


  void scene::pre_render()
  {
    LOG_TRACE("Scene: pre-render");

    assert(objects.size() > 0);
    for (hittable* object : objects)
    {
      assert(object != nullptr);
      object->pre_render();
    }
  }
}