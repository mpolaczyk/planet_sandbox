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
#include "hittables/light.h"
#include "math/colors.h"
#include "object/object_registry.h"

namespace engine
{
  void vserialize_object::visit(amaterial& object) const
  {
    j["color"] = fpersistence::serialize(object.color);
    j["emitted_color"] = fpersistence::serialize(object.emitted_color);
    j["gloss_color"] = fpersistence::serialize(object.gloss_color);
    j["smoothness"] = object.smoothness;
    j["gloss_probability"] = object.gloss_probability;
    j["refraction_probability"] = object.refraction_probability;
    j["refraction_index"] = object.refraction_index;
  }
  void vserialize_object::visit(atexture& object) const
  {
    j["desired_channels"] = object.desired_channels;
    j["img_file_name"] = object.img_file_name;
  }
  void vserialize_object::visit(astatic_mesh& object) const
  {
    j["obj_file_name"] = object.obj_file_name;
  }
  void vserialize_object::visit(avertex_shader& object) const
  {
    j["shader_file_name"] = object.shader_file_name;
    j["entrypoint"] = object.entrypoint;
    j["target"] = object.target;
  }
  void vserialize_object::visit(apixel_shader& object) const
  {
    j["shader_file_name"] = object.shader_file_name;
    j["entrypoint"] = object.entrypoint;
    j["target"] = object.target;
  }
  
  void vserialize_object::visit(hhittable_base& object) const
  {
    j["origin"] = fpersistence::serialize(object.origin);
    j["scale"] = fpersistence::serialize(object.scale);
    j["rotation"] = fpersistence::serialize(object.rotation);
    j["material_asset"] = fpersistence::serialize(object.material_asset_ptr);
    j["custom_display_name"] = object.get_display_name();
  }
  void vserialize_object::visit(hscene& object) const
  {
    object.hhittable_base::accept(vserialize_object(j));
    nlohmann::json jarr = nlohmann::json::array();
    for (hhittable_base* h : object.objects)
    {
      nlohmann::json jj;
      jj["class_name"] = h->get_class()->get_class_name();
      h->accept(vserialize_object(jj));
      jarr.push_back(jj);
    }
    j["objects"] = jarr;
    j["ambient_light_color"] = fpersistence::serialize(object.ambient_light_color);
    j["clear_color"] = fpersistence::serialize(object.clear_color);
    j["renderer_config"] = fpersistence::serialize(object.renderer_config);
    j["camera_config"] = fpersistence::serialize(object.camera_config);
  }
  void vserialize_object::visit(hstatic_mesh& object) const
  {
    object.hhittable_base::accept(vserialize_object(j));
    j["mesh_asset"] = fpersistence::serialize(object.mesh_asset_ptr);
  }
  void vserialize_object::visit(hsphere& object) const
  {
    object.hhittable_base::accept(vserialize_object(j));
    j["radius"] = object.radius;
  }
  void vserialize_object::visit(hlight& object) const
  {
    object.hhittable_base::accept(vserialize_object(j));
    j["properties"] = fpersistence::serialize(object.properties);
  }
  
  void vdeserialize_object::visit(amaterial& object) const
  {
    nlohmann::json jcolor;
    if (TRY_PARSE(nlohmann::json, j, "color", jcolor)) { fpersistence::deserialize(jcolor, object.color); }
    assert(fcolors::is_valid(object.color));

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
  void vdeserialize_object::visit(atexture& object) const
  {
    TRY_PARSE(int, j, "desired_channels", object.desired_channels);

    TRY_PARSE(std::string, j, "img_file_name", object.img_file_name);
  }
  void vdeserialize_object::visit(astatic_mesh& object) const
  {
    TRY_PARSE(std::string, j, "obj_file_name", object.obj_file_name);
  }
  void vdeserialize_object::visit(avertex_shader& object) const
  {
    TRY_PARSE(std::string, j, "shader_file_name", object.shader_file_name);
    TRY_PARSE(std::string, j, "entrypoint", object.entrypoint);
    TRY_PARSE(std::string, j, "target", object.target);
  }
  void vdeserialize_object::visit(apixel_shader& object) const
  {
    TRY_PARSE(std::string, j, "shader_file_name", object.shader_file_name);
    TRY_PARSE(std::string, j, "entrypoint", object.entrypoint);
    TRY_PARSE(std::string, j, "target", object.target);
  }
  
  void vdeserialize_object::visit(hhittable_base& object) const
  {
    nlohmann::json jorigin;
    if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { fpersistence::deserialize(jorigin, object.origin); }
    nlohmann::json jscale;
    if (TRY_PARSE(nlohmann::json, j, "scale", jscale)) { fpersistence::deserialize(jscale, object.scale); }
    nlohmann::json jrotation;
    if (TRY_PARSE(nlohmann::json, j, "rotation", jrotation)) { fpersistence::deserialize(jrotation, object.rotation); }
    nlohmann::json jmaterial;
    if (TRY_PARSE(nlohmann::json, j, "material_asset", jmaterial)) { fpersistence::deserialize(jmaterial, object.material_asset_ptr); }

    std::string name;
    if (TRY_PARSE(std::string, j, "custom_display_name", name)) { object.set_display_name(name); }
  }
  void vdeserialize_object::visit(hscene& object) const
  {
    object.hhittable_base::accept(vdeserialize_object(j));

    nlohmann::json jambient_light_color;
    if (TRY_PARSE(nlohmann::json, j, "ambient_light_color", jambient_light_color)) { fpersistence::deserialize(jambient_light_color, object.ambient_light_color); }

    nlohmann::json jclear_color;
    if (TRY_PARSE(nlohmann::json, j, "clear_color", jclear_color)) { fpersistence::deserialize(jclear_color, object.clear_color); }

    nlohmann::json jrenderer_config;
    if (TRY_PARSE(nlohmann::json, j, "renderer_config", jrenderer_config)) { fpersistence::deserialize(jrenderer_config, object.renderer_config); }

    nlohmann::json jcamera_conf;
    if (TRY_PARSE(nlohmann::json, j, "camera_config", jcamera_conf)) { fpersistence::deserialize(jcamera_conf, object.camera_config); }
    
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
            obj->accept(vdeserialize_object(jobj));
            object.objects.push_back(obj);
          }
        }
      }
    }
  }
  void vdeserialize_object::visit(hstatic_mesh& object) const
  {
    object.hhittable_base::accept(vdeserialize_object(j));
    nlohmann::json jmesh;
    if (TRY_PARSE(nlohmann::json, j, "mesh_asset", jmesh)) { fpersistence::deserialize(jmesh, object.mesh_asset_ptr); }
  }
  void vdeserialize_object::visit(hsphere& object) const
  {
    object.hhittable_base::accept(vdeserialize_object(j));
    TRY_PARSE(float, j, "radius", object.radius);
   }
  void vdeserialize_object::visit(hlight& object) const
  {
    object.hhittable_base::accept(vdeserialize_object(j));
    nlohmann::json jproperties;
    if (TRY_PARSE(nlohmann::json, j, "properties", jproperties)) { fpersistence::deserialize(jproperties, object.properties); }
  }
}
