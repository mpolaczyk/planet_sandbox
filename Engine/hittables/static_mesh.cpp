
#include <sstream>

#include "hittables/static_mesh.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "engine/hash.h"
#include "profile/stats.h"
#include "math/math.h"
#include "math/triangle_face.h"
#include "object/object_registry.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(static_mesh, hittable, Static mesh)
  OBJECT_DEFINE_SPAWN(static_mesh)

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


  inline uint32_t static_mesh::get_hash() const
  {
    uint32_t a = hash::combine(hittable::get_hash(), hash::get(origin), hash::get(extent), hash::get(rotation));
    uint32_t b = hash::combine(hash::get(scale), hash::get(material_asset_ptr.get_name().c_str()));
    return hash::combine(a, b);
  }

  static_mesh* static_mesh::clone() const
  {
    return REG.copy_shallow<static_mesh>(this);
  }

  void static_mesh::load_resources()
  {
    mesh_asset_ptr.get();
  }


  void static_mesh::pre_render()
  {
    // Create a temporary object and modify it, reuse it next frame
    std::ostringstream oss;
    oss << "temp_" << mesh_asset_ptr.get_name().c_str() << "_hittable_id_" << get_runtime_id();

    if (runtime_asset_ptr == nullptr)
    {
      runtime_asset_ptr = REG.copy_shallow<static_mesh_asset>(mesh_asset_ptr.get());
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