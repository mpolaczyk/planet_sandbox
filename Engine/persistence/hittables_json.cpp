
#include "hittables/hittables.h"
#include "hittables/static_mesh.h"
#include "hittables/sphere.h"
#include "hittables/scene.h"

#include "persistence/vec3_json.h"
#include "persistence/hittables_json.h"
#include "persistence/assets_json.h"

#include "nlohmann/json.hpp"
#include "engine/log.h"
#include "object/object_registry.h"

namespace engine
{
  nlohmann::json hittable_serializer::serialize(const hittable* value)
  {
    assert(value != nullptr);
    nlohmann::json j;
    j["material_asset"] = soft_asset_ptr_base_serializer::serialize(&value->material_asset_ptr);
    j["custom_display_name"] = get_object_registry()->get_custom_display_name(value->get_runtime_id());
    return j;
  }

  nlohmann::json scene_serializer::serialize(const scene* value)
  {
    assert(value != nullptr);
    nlohmann::json j;
    j = hittable_serializer::serialize(value);
    nlohmann::json jarr = nlohmann::json::array();
    for (const hittable* object : value->objects)
    {
      const class_object* class_o = object->get_class();
      std::string class_name = class_o->class_name;

      if (class_o == scene::get_class_static())
      {
        jarr.push_back(scene_serializer::serialize(static_cast<const scene*>(object)));
      }
      else if (class_o == sphere::get_class_static())
      {
        jarr.push_back(sphere_serializer::serialize(static_cast<const sphere*>(object)));
      }
      else if (class_o == static_mesh::get_class_static())
      {
        jarr.push_back(static_mesh_serializer::serialize(static_cast<const static_mesh*>(object)));
      }
      else
      {
        LOG_ERROR("Undefined hittable to serialize: {0}", class_name);
      }
      j["class_name"] = class_name;
    }
    j["objects"] = jarr;
    return j;
  }

  nlohmann::json static_mesh_serializer::serialize(const static_mesh* value)
  {
    assert(value != nullptr);
    nlohmann::json j;
    j = hittable_serializer::serialize(value);
    j["origin"] = vec3_serializer::serialize(value->origin);
    j["scale"] = vec3_serializer::serialize(value->scale);
    j["rotation"] = vec3_serializer::serialize(value->rotation);
    j["mesh_asset"] = soft_asset_ptr_base_serializer::serialize(&value->mesh_asset_ptr);
    return j;
  }

  nlohmann::json sphere_serializer::serialize(const sphere* value)
  {
    assert(value != nullptr);
    nlohmann::json j;
    j = hittable_serializer::serialize(value);
    j["radius"] = value->radius;
    j["origin"] = vec3_serializer::serialize(value->origin);
    return j;
  }

  
  void hittable_serializer::deserialize(const nlohmann::json& j, hittable* out_value)
  {
    assert(out_value != nullptr); 

    nlohmann::json jmaterial;
    if (TRY_PARSE(nlohmann::json, j, "material_asset", jmaterial)) { soft_asset_ptr_base_serializer::deserialize(jmaterial, &out_value->material_asset_ptr); }

    std::string name;
    if (TRY_PARSE(std::string, j, "custom_display_name", name)) { get_object_registry()->set_custom_display_name(out_value->get_runtime_id(), name); }
  }

  void scene_serializer::deserialize(const nlohmann::json& j, scene* out_value)
  {
    assert(out_value != nullptr);

    nlohmann::json jobjects;
    if (TRY_PARSE(nlohmann::json, j, "objects", jobjects))
    {
      for (const auto& jobj : jobjects)
      {
        std::string class_name;
        if (TRY_PARSE(std::string, jobj, "class_name", class_name))
        {
          const class_object* class_o = get_object_registry()->find_class(class_name);
          hittable* obj = class_o->spawn_instance<hittable>();
          if (obj != nullptr)
          {
            if (class_o == scene::get_class_static())
            {
              scene_serializer::deserialize(jobj, static_cast<scene*>(obj));
            }
            else if (class_o == sphere::get_class_static())
            {
              sphere_serializer::deserialize(jobj, static_cast<sphere*>(obj));
            }
            else if (class_o == static_mesh::get_class_static())
            {
              static_mesh_serializer::deserialize(jobj, static_cast<static_mesh*>(obj));
            }
            else
            {
              LOG_ERROR("Unable to deserialize hittable of type: {0}", class_o->class_name);
            }
            out_value->objects.push_back(obj);
          }
        }
      }
    }
  }

  void static_mesh_serializer::deserialize(const nlohmann::json& j, static_mesh* out_value)
  {
    assert(out_value != nullptr);
    hittable_serializer::deserialize(j, out_value);

    nlohmann::json jorigin;
    if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { out_value->origin = vec3_serializer::deserialize(jorigin); }
    nlohmann::json jscale;
    if (TRY_PARSE(nlohmann::json, j, "scale", jscale)) { out_value->scale = vec3_serializer::deserialize(jscale); }
    nlohmann::json jrotation;
    if (TRY_PARSE(nlohmann::json, j, "rotation", jrotation)) { out_value->rotation = vec3_serializer::deserialize(jrotation); }

    nlohmann::json jmesh;
    if (TRY_PARSE(nlohmann::json, j, "mesh_asset", jmesh)) { soft_asset_ptr_base_serializer::deserialize(jmesh, &out_value->mesh_asset_ptr); }
  };

  void sphere_serializer::deserialize(const nlohmann::json& j, sphere* out_value)
  {
    assert(out_value != nullptr);
    hittable_serializer::deserialize(j, out_value);

    TRY_PARSE(float, j, "radius", out_value->radius);

    nlohmann::json jorigin;
    if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { out_value->origin = vec3_serializer::deserialize(jorigin); }
  }
}