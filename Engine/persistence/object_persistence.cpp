#include <cassert>

#include "nlohmann/json.hpp"

#include "persistence/object_persistence.h"
#include "persistence/persistence_helper.h"
#include "persistence/persistence.h"

#include "assets/material.h"
#include "assets/mesh.h"
#include "assets/texture.h"
#include "assets/vertex_shader.h"
#include "assets/pixel_shader.h"

#include "hittables/hittables.h"
#include "hittables/static_mesh.h"
#include "hittables/sphere.h"
#include "hittables/scene.h"

#include "engine/log.h"
#include "math/colors.h"
#include "object/object_registry.h"

namespace engine
{
  void serialize_object::visit(class amaterial& object) const
  {
    j["color"] = fpersistence::serialize(object.color);
    j["emitted_color"] = fpersistence::serialize(object.emitted_color);
    j["gloss_color"] = fpersistence::serialize(object.gloss_color);
    j["is_light"] = object.is_light;
    j["smoothness"] = object.smoothness;
    j["gloss_probability"] = object.gloss_probability;
    j["refraction_probability"] = object.refraction_probability;
    j["refraction_index"] = object.refraction_index;
  }
  void serialize_object::visit(class atexture& object) const
  {
    j["desired_channels"] = object.desired_channels;
    j["img_file_name"] = object.img_file_name;
  }
  void serialize_object::visit(class astatic_mesh& object) const
  {
    j["obj_file_name"] = object.obj_file_name;
  }
  void serialize_object::visit(class avertex_shader& object) const
  {
    j["shader_file_name"] = object.shader_file_name;
    j["entrypoint"] = object.entrypoint;
    j["target"] = object.target;
  }
  void serialize_object::visit(class apixel_shader& object) const
  {
    j["shader_file_name"] = object.shader_file_name;
    j["entrypoint"] = object.entrypoint;
    j["target"] = object.target;
  }
  
  void serialize_object::visit(class hhittable_base& object) const
  {
    j["material_asset"] = fpersistence::serialize(object.material_asset_ptr);
    j["custom_display_name"] = REG.get_custom_display_name(object.get_runtime_id());
  }
  void serialize_object::visit(class hscene& object) const
  {
    object.hhittable_base::accept(serialize_object(j));
    nlohmann::json jarr = nlohmann::json::array();
    for (hhittable_base* h : object.objects)
    {
      nlohmann::json jj;
      jj["class_name"] = h->get_class()->get_class_name();
      h->accept(serialize_object(jj));
      jarr.push_back(jj);
    }
    j["class_name"] = hscene::get_class_static()->get_class_name();
    j["objects"] = jarr;
  }
  void serialize_object::visit(class hstatic_mesh& object) const
  {
    object.hhittable_base::accept(serialize_object(j));
    j["origin"] = fpersistence::serialize(object.origin);
    j["scale"] = fpersistence::serialize(object.scale);
    j["rotation"] = fpersistence::serialize(object.rotation);
    j["mesh_asset"] = fpersistence::serialize(object.mesh_asset_ptr);
  }
  void serialize_object::visit(class hsphere& object) const
  {
    object.hhittable_base::accept(serialize_object(j));
    j["radius"] = object.radius;
    j["origin"] = fpersistence::serialize(object.origin);
  }
  
  void deserialize_object::visit(class amaterial& object) const
  {
    nlohmann::json jcolor;
    if (TRY_PARSE(nlohmann::json, j, "color", jcolor)) { fpersistence::deserialize(jcolor, object.color); }
    assert(fcolors::is_valid(object.color));

    TRY_PARSE(bool, j, "is_light", object.is_light);

    nlohmann::json jemitted_color;
    if (TRY_PARSE(nlohmann::json, j, "emitted_color", jemitted_color)) { fpersistence::deserialize(jemitted_color, object.emitted_color); }
    assert(fcolors::is_valid(object.emitted_color));

    TRY_PARSE(float, j, "smoothness", object.smoothness);
    assert(object.smoothness >= 0.0f && object.smoothness <= 1.0f);

    nlohmann::json jgloss_color;
    if (TRY_PARSE(nlohmann::json, j, "gloss_color", jgloss_color)) { fpersistence::deserialize(jgloss_color, object.gloss_color); }
    assert(fcolors::is_valid(object.gloss_color));

    TRY_PARSE(float, j, "gloss_probability", object.gloss_probability);
    assert(object.gloss_probability >= 0.0f && object.gloss_probability <= 1.0f);

    TRY_PARSE(float, j, "refraction_probability", object.refraction_probability);
    assert(object.refraction_probability >= 0.0f && object.refraction_probability <= 1.0f);
    TRY_PARSE(float, j, "refraction_index", object.refraction_index);
  }
  void deserialize_object::visit(class atexture& object) const
  {
    TRY_PARSE(int, j, "desired_channels", object.desired_channels);

    TRY_PARSE(std::string, j, "img_file_name", object.img_file_name);
  }
  void deserialize_object::visit(class astatic_mesh& object) const
  {
    TRY_PARSE(std::string, j, "obj_file_name", object.obj_file_name);
  }
  void deserialize_object::visit(class avertex_shader& object) const
  {
    TRY_PARSE(std::string, j, "shader_file_name", object.shader_file_name);
    TRY_PARSE(std::string, j, "entrypoint", object.entrypoint);
    TRY_PARSE(std::string, j, "target", object.target);
  }
  void deserialize_object::visit(class apixel_shader& object) const
  {
    TRY_PARSE(std::string, j, "shader_file_name", object.shader_file_name);
    TRY_PARSE(std::string, j, "entrypoint", object.entrypoint);
    TRY_PARSE(std::string, j, "target", object.target);
  }
  
  void deserialize_object::visit(class hhittable_base& object) const
  {
    nlohmann::json jmaterial;
    if (TRY_PARSE(nlohmann::json, j, "material_asset", jmaterial)) { fpersistence::deserialize(jmaterial, object.material_asset_ptr); }

    std::string name;
    if (TRY_PARSE(std::string, j, "custom_display_name", name)) { REG.set_custom_display_name(object.get_runtime_id(), name); }
  }
  void deserialize_object::visit(class hscene& object) const
  {
    object.hhittable_base::accept(deserialize_object(j));
    
    nlohmann::json jobjects;
    if (TRY_PARSE(nlohmann::json, j, "objects", jobjects))
    {
      for (const auto& jobj : jobjects)
      {
        std::string class_name;
        if (TRY_PARSE(std::string, jobj, "class_name", class_name))
        {
          const oclass_object* class_o = REG.find_class(class_name);
          hhittable_base* obj = REG.spawn_from_class<hhittable_base>(class_o);
          if (obj != nullptr)
          {
            obj->accept(deserialize_object(jobj));
            object.objects.push_back(obj);
          }
        }
      }
    }
  }
  void deserialize_object::visit(class hstatic_mesh& object) const
  {
    object.hhittable_base::accept(deserialize_object(j));
    nlohmann::json jorigin;
    if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { fpersistence::deserialize(jorigin, object.origin); }
    nlohmann::json jscale;
    if (TRY_PARSE(nlohmann::json, j, "scale", jscale)) { fpersistence::deserialize(jscale, object.scale); }
    nlohmann::json jrotation;
    if (TRY_PARSE(nlohmann::json, j, "rotation", jrotation)) { fpersistence::deserialize(jrotation, object.rotation); }
    nlohmann::json jmesh;
    if (TRY_PARSE(nlohmann::json, j, "mesh_asset", jmesh)) { fpersistence::deserialize(jmesh, object.mesh_asset_ptr); }
  }
  void deserialize_object::visit(class hsphere& object) const
  {
    object.hhittable_base::accept(deserialize_object(j));
    TRY_PARSE(float, j, "radius", object.radius);
    nlohmann::json jorigin;
    if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { fpersistence::deserialize(jorigin, object.origin); }
   }
}
