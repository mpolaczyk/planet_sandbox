#include <cassert>

#include "nlohmann/json.hpp"

#include "persistence/object_persistence.h"

#include "assets/material.h"
#include "assets/mesh.h"
#include "assets/texture.h"
#include "engine/log.h"

#include "hittables/hittables.h"
#include "hittables/static_mesh.h"
#include "hittables/sphere.h"
#include "hittables/scene.h"

#include "persistence/persistence_helper.h"
#include "persistence/persistence.h"

#include "math/colors.h"
#include "object/object_registry.h"

namespace engine
{
  void serialize_object::visit(class material_asset& object) const
  {
    j["color"] = persistence::serialize(object.color);
    j["emitted_color"] = persistence::serialize(object.emitted_color);
    j["gloss_color"] = persistence::serialize(object.gloss_color);
    j["is_light"] = object.is_light;
    j["smoothness"] = object.smoothness;
    j["gloss_probability"] = object.gloss_probability;
    j["refraction_probability"] = object.refraction_probability;
    j["refraction_index"] = object.refraction_index;
  }
  void serialize_object::visit(class texture_asset& object) const
  {
    j["width"] = object.width;
    j["height"] = object.height;
    j["img_file_name"] = object.img_file_name;
  }
  void serialize_object::visit(class static_mesh_asset& object) const
  {
    j["shape_index"] = object.shape_index;
    j["obj_file_name"] = object.obj_file_name;
  }
  
  void serialize_object::visit(class hittable& object) const
  {
    j["material_asset"] = persistence::serialize(object.material_asset_ptr);
    j["custom_display_name"] = REG.get_custom_display_name(object.get_runtime_id());
  }
  void serialize_object::visit(class scene& object) const
  {
    object.hittable::accept(serialize_object(j));
    nlohmann::json jarr = nlohmann::json::array();
    for (hittable* h : object.objects)
    {
      j["class_name"] = h->get_class()->class_name;
      h->accept(serialize_object(j));
    }
    j["objects"] = jarr;
  }
  void serialize_object::visit(class static_mesh& object) const
  {
    object.hittable::accept(serialize_object(j));
    j["origin"] = persistence::serialize(object.origin);
    j["scale"] = persistence::serialize(object.scale);
    j["rotation"] = persistence::serialize(object.rotation);
    j["mesh_asset"] = persistence::serialize(object.mesh_asset_ptr);
  }
  void serialize_object::visit(class sphere& object) const
  {
    object.hittable::accept(serialize_object(j));
    j["radius"] = object.radius;
    j["origin"] = persistence::serialize(object.origin);
  }
  
  void deserialize_object::visit(class material_asset& object) const
  {
    nlohmann::json jcolor;
    if (TRY_PARSE(nlohmann::json, j, "color", jcolor)) { persistence::deserialize(jcolor, object.color); }
    assert(colors::is_valid(object.color));

    TRY_PARSE(bool, j, "is_light", object.is_light);

    nlohmann::json jemitted_color;
    if (TRY_PARSE(nlohmann::json, j, "emitted_color", jemitted_color)) { persistence::deserialize(jemitted_color, object.emitted_color); }
    assert(colors::is_valid(object.emitted_color));

    TRY_PARSE(float, j, "smoothness", object.smoothness);
    assert(object.smoothness >= 0.0f && object.smoothness <= 1.0f);

    nlohmann::json jgloss_color;
    if (TRY_PARSE(nlohmann::json, j, "gloss_color", jgloss_color)) { persistence::deserialize(jgloss_color, object.gloss_color); }
    assert(colors::is_valid(object.gloss_color));

    TRY_PARSE(float, j, "gloss_probability", object.gloss_probability);
    assert(object.gloss_probability >= 0.0f && object.gloss_probability <= 1.0f);

    TRY_PARSE(float, j, "refraction_probability", object.refraction_probability);
    assert(object.refraction_probability >= 0.0f && object.refraction_probability <= 1.0f);
    TRY_PARSE(float, j, "refraction_index", object.refraction_index);
  }
  void deserialize_object::visit(class texture_asset& object) const
  {
    TRY_PARSE(int, j, "width", object.width);
    TRY_PARSE(int, j, "height", object.height);

    TRY_PARSE(std::string, j, "img_file_name", object.img_file_name);
  }
  void deserialize_object::visit(class static_mesh_asset& object) const
  {
    TRY_PARSE(int, j, "shape_index", object.shape_index);

    TRY_PARSE(std::string, j, "obj_file_name", object.obj_file_name);
  }

  void deserialize_object::visit(class hittable& object) const
  {
    nlohmann::json jmaterial;
    if (TRY_PARSE(nlohmann::json, j, "material_asset", jmaterial)) { persistence::deserialize(jmaterial, object.material_asset_ptr); }

    std::string name;
    if (TRY_PARSE(std::string, j, "custom_display_name", name)) { REG.set_custom_display_name(object.get_runtime_id(), name); }
  }
  void deserialize_object::visit(class scene& object) const
  {
    object.hittable::accept(deserialize_object(j));
    
    nlohmann::json jobjects;
    if (TRY_PARSE(nlohmann::json, j, "objects", jobjects))
    {
      for (const auto& jobj : jobjects)
      {
        std::string class_name;
        if (TRY_PARSE(std::string, jobj, "class_name", class_name))
        {
          const class_object* class_o = REG.find_class(class_name);
          hittable* obj = class_o->spawn_instance<hittable>();
          if (obj != nullptr)
          {
            obj->accept(deserialize_object(jobj));
            object.objects.push_back(obj);
          }
        }
      }
    }
  }
  void deserialize_object::visit(class static_mesh& object) const
  {
    object.hittable::accept(deserialize_object(j));
    nlohmann::json jorigin;
    if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { persistence::deserialize(jorigin, object.origin); }
    nlohmann::json jscale;
    if (TRY_PARSE(nlohmann::json, j, "scale", jscale)) { persistence::deserialize(jscale, object.scale); }
    nlohmann::json jrotation;
    if (TRY_PARSE(nlohmann::json, j, "rotation", jrotation)) { persistence::deserialize(jrotation, object.rotation); }
    nlohmann::json jmesh;
    if (TRY_PARSE(nlohmann::json, j, "mesh_asset", jmesh)) { persistence::deserialize(jmesh, object.mesh_asset_ptr); }
  }
  void deserialize_object::visit(class sphere& object) const
  {
    object.hittable::accept(deserialize_object(j));
    TRY_PARSE(float, j, "radius", object.radius);
    nlohmann::json jorigin;
    if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { persistence::deserialize(jorigin, object.origin); }
   }
}
