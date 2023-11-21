#include <sstream>

#include "hittables/hittables.h"

#include "math/aabb.h"
#include "math/onb.h"
#include "assets/material.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"

namespace engine
{
  int hittable::last_id = 0;

  scene::~scene()
  {
    for (hittable* obj : objects)
    {
      delete obj;
    }
  }


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

  bool static_mesh::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
  {
    if (runtime_asset_ptr == nullptr || runtime_asset_ptr->faces.size() == 0)
    {
      return false;
    }
    const std::vector<triangle_face>& faces = runtime_asset_ptr->faces;

    bool result = false;
    hit_record best_hit;
    int hits = 0;
    bool can_refract = material_asset_ptr.get()->refraction_probability > 0.0f;
    // backface triangles can be dropped if the material is not refracting.
    // 
    // TODO: check dot product of face normal and ray direction here instead of dropping inside function
    // This required mesh with normals which does not work right now.

    // Multiple triangles can intersect, find the closest one.
    for (int i = 0; i < faces.size(); i++)
    {
      const triangle_face* face = &faces[i];
      hit_record h;
      if (math::ray_triangle(in_ray, t_min, t_max, face, h, !can_refract))
      {
        if (hits == 0)
        {
          best_hit = h;
          best_hit.face_id = i;
          hits++;
        }
        else if (h.t < best_hit.t)
        {
          best_hit = h;
          best_hit.face_id = i;
        }
      }
    }
    if (hits > 0)
    {
      out_hit = best_hit;
      out_hit.material_ptr = material_asset_ptr.get();
      return true;
    }

    return false;
  }

  bool sphere::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
  {
    vec3 oc = in_ray.origin - origin;
    float a = math::length_squared(in_ray.direction);
    float half_b = math::dot(oc, in_ray.direction);
    float c = math::length_squared(oc) - radius * radius;

    float delta = half_b * half_b - a * c;
    if (delta < 0.0f)
    {
      return false;
    }

    // Find the nearest root that lies in the acceptable range.
    float sqrtd = sqrt(delta);
    float root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root)
    {
      root = (-half_b + sqrtd) / a;
      if (root < t_min || t_max < root)
      {
        return false;
      }
    }

    out_hit.t = root;
    out_hit.p = in_ray.at(out_hit.t);
    out_hit.material_ptr = material_asset_ptr.get();

    // Normal always against the ray
    vec3 outward_normal = (out_hit.p - origin) / radius;
    out_hit.front_face = math::flip_normal_if_front_face(in_ray.direction, outward_normal, out_hit.normal);
    math::get_sphere_uv(outward_normal, out_hit.u, out_hit.v);
    return true;
  }


  void hittable::get_name(std::string& out_name, bool with_params) const
  {
    std::string base_name = hittable_type_names[(int)type];

    std::ostringstream oss;
    oss << "[" << id << "]" << "/" << base_name << "/" << material_asset_ptr.get_name();
    out_name = oss.str();
  }

  void scene::get_name(std::string& out_name, bool with_params) const
  {
    out_name = hittable_type_names[(int)hittable_type::scene];
  }

  void static_mesh::get_name(std::string& out_name, bool with_params) const
  {
    std::string base_name;
    hittable::get_name(base_name);
    if (with_params)
    {
      std::ostringstream oss;
      oss << base_name << "/" << mesh_asset_ptr.get_name();
      out_name = oss.str();
    }
    else
    {
      out_name = base_name;
    }
  }

  void sphere::get_name(std::string& out_name, bool with_params) const
  {
    std::string base_name;
    hittable::get_name(base_name);
    if (with_params)
    {
      std::ostringstream oss;
      oss << base_name << "/" << radius;
      out_name = oss.str();
    }
    else
    {
      out_name = base_name;
    }
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

  bool static_mesh::get_bounding_box(aabb& out_box) const
  {
    vec3 mi(0.0f);  // minimum corner
    vec3 ma(0.0f);  // maximum corner
    if (runtime_asset_ptr != nullptr)
    {
      for (const triangle_face& f : runtime_asset_ptr->faces)
      {
        const vec3& fv = f.vertices[0];
        if (fv.x < mi.x) { mi.x = fv.x; }
        if (fv.y < mi.y) { mi.y = fv.y; }
        if (fv.z < mi.z) { mi.z = fv.z; }
        if (fv.x > ma.x) { ma.x = fv.x; }
        if (fv.y > ma.y) { ma.y = fv.y; }
        if (fv.z > ma.z) { ma.z = fv.z; }
      }
    }
    out_box = aabb(mi, ma);
    return true;
  }

  bool sphere::get_bounding_box(aabb& out_box) const
  {
    out_box = aabb(origin - radius, origin + radius);
    return true;
  }


  inline uint32_t hittable::get_hash() const
  {
    return hash::combine(hash::get(material_asset_ptr.get_name().c_str()), (int)type);
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

  inline uint32_t static_mesh::get_hash() const
  {
    uint32_t a = hash::combine(hittable::get_hash(), hash::get(origin), hash::get(extent), hash::get(rotation));
    uint32_t b = hash::combine(hash::get(scale), hash::get(material_asset_ptr.get_name().c_str()));
    return hash::combine(a, b);
  }

  inline uint32_t sphere::get_hash() const
  {
    return hash::combine(hittable::get_hash(), hash::get(origin), hash::get(radius));
  }


  scene* scene::clone() const
  {
    scene* ans = new scene();   // FIX
    *ans = *this;
    ans->objects.clear();
    // Deep copy
    for (const hittable* obj : objects)
    {
      hittable* new_obj = obj->clone();
      ans->objects.push_back(new_obj);
    }
    return ans;
  }

  static_mesh* static_mesh::clone() const
  {
    static_mesh* ans = new static_mesh();  // FIX
    *ans = *this;
    return ans;
  }

  sphere* sphere::clone() const
  {
    sphere* ans = new sphere();  // FIX
    *ans = *this;
    return ans;
  }


  void hittable::load_resources()
  {
    // Materials are loaded on editor startup because I need to show all of them in UI
    // This is here for consistence:
    material_asset_ptr.get();
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

  void static_mesh::load_resources()
  {
    mesh_asset_ptr.get();
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

  void static_mesh::pre_render()
  {
    // Create a temporary object and modify it, reuse it next frame
    std::ostringstream oss;
    oss << "temp_" << mesh_asset_ptr.get_name().c_str() << "_hittable_id_" << id;
    runtime_asset_ptr = get_object_registry()->find<static_mesh_asset>(oss.str());
    if (runtime_asset_ptr == nullptr)
    {
      if (get_object_registry()->find<static_mesh_asset>(mesh_asset_ptr.get_name()) != nullptr)
      {
        runtime_asset_ptr = get_object_registry()->clone<static_mesh_asset>(mesh_asset_ptr.get()->get_runtime_id(), oss.str());
      }
      else
      {
        return;
      }
    }

    float y_rotation = rotation.y / 180.0f * math::pi;

    const std::vector<triangle_face>& mesh_faces = mesh_asset_ptr.get()->faces;
    std::vector<triangle_face>& runtime_faces = runtime_asset_ptr->faces;
    runtime_faces.clear();
    runtime_faces = mesh_faces;

    // Translate asset vertices to the world coordinates
    for (int f = 0; f < mesh_faces.size(); f++)
    {
      const triangle_face& in = mesh_faces[f];

      // Calculate world location for each vertex
      for (size_t vi = 0; vi < 3; ++vi)
      {
        runtime_faces[f].vertices[vi].x = (cosf(y_rotation) * in.vertices[vi].x - sinf(y_rotation) * in.vertices[vi].z) * scale.x + origin.x;
        runtime_faces[f].vertices[vi].y = in.vertices[vi].y * scale.x + origin.y;
        runtime_faces[f].vertices[vi].z = (sinf(y_rotation) * in.vertices[vi].x + cosf(y_rotation) * in.vertices[vi].z) * scale.x + origin.z;
      }

      // Vertex normals
      for (size_t ni = 0; ni < 3; ++ni)
      {
        runtime_faces[f].normals[ni].x = in.normals[ni].x;
        runtime_faces[f].normals[ni].y = in.normals[ni].y;
        runtime_faces[f].normals[ni].z = in.normals[ni].z;
      }

      //for (size_t ni = 0; ni < 3; ++ni)
      //{
      //  runtime_faces[f].normals[ni].x = (cosf(y_rotation) * in.normals[ni].x - sinf(y_rotation) * in.normals[ni].z) * scale.x + origin.x;
      //  runtime_faces[f].normals[ni].y = in.normals[ni].y * scale.x + origin.y;
      //  runtime_faces[f].normals[ni].z = (sinf(y_rotation) * in.normals[ni].x + cosf(y_rotation) * in.normals[ni].z) * scale.x + origin.z;
      //}
    }
  }
}