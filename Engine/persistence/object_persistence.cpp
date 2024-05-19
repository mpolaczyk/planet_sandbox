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

#include "renderer/renderer_base.h"
#include "renderers/gpu_forward_sync.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "object/object_registry.h"
#include "renderers/gpu_deferred_sync.h"

namespace engine
{
  void vserialize_object::visit(amaterial& object) const
  {
    j["properties"] = fpersistence::serialize(object.properties);
    j["texture_asset"] = fpersistence::serialize(object.texture_asset_ptr);
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
    j["custom_display_name"] = object.get_display_name();
  }

  void vserialize_object::visit(hscene& object) const
  {
    {
      nlohmann::json jobjects = nlohmann::json::array();
      for(hhittable_base* h : object.objects)
      {
        nlohmann::json jj;
        jj["class_name"] = h->get_class()->get_class_name();
        h->accept(vserialize_object(jj));
        jobjects.push_back(jj);
      }
      j["objects"] = jobjects;
    }
    {
      nlohmann::json jrenderer;
      jrenderer["class_name"] = object.renderer->get_class()->get_class_name();
      object.renderer->accept(vserialize_object(jrenderer));
      j["renderer"] = jrenderer;
    }
    j["ambient_light_color"] = fpersistence::serialize(object.ambient_light_color);
    j["clear_color"] = fpersistence::serialize(object.clear_color);
    j["camera_config"] = fpersistence::serialize(object.camera_config);
  }

  void vserialize_object::visit(hstatic_mesh& object) const
  {
    object.hhittable_base::accept(vserialize_object(j));
    j["mesh_asset"] = fpersistence::serialize(object.mesh_asset_ptr);
    j["material_asset"] = fpersistence::serialize(object.material_asset_ptr);
  }

  void vserialize_object::visit(hsphere& object) const
  {
    object.hhittable_base::accept(vserialize_object(j));
    j["radius"] = object.radius;
  }

  void vserialize_object::visit(hlight& object) const
  {
    // Only part of hittable
    j["origin"] = fpersistence::serialize(object.origin);
    j["custom_display_name"] = object.get_display_name();

    j["properties"] = fpersistence::serialize(object.properties);
  }

  void vserialize_object::visit_rrenderer_base(rrenderer_base& object) const
  {
    j["output_width"] = object.output_width;
    j["output_height"] = object.output_height;
    j["default_material_asset"] = fpersistence::serialize(object.default_material_asset);
  }

  void vserialize_object::visit(rgpu_forward_sync& object) const
  {
    visit_rrenderer_base(object);
    j["pixel_shader_asset"] = fpersistence::serialize(object.pixel_shader_asset);
    j["vertex_shader_asset"] = fpersistence::serialize(object.vertex_shader_asset);
  }

  void vserialize_object::visit(rgpu_deferred_sync& object) const
  {
    visit_rrenderer_base(object);
    j["gbuffer_pixel_shader_asset"] = fpersistence::serialize(object.gbuffer_pixel_shader_asset);
    j["gbuffer_vertex_shader_asset"] = fpersistence::serialize(object.gbuffer_vertex_shader_asset);
  }

  void vdeserialize_object::visit(amaterial& object) const
  {
    nlohmann::json jproperties;
    if(TRY_PARSE(nlohmann::json, j, "properties", jproperties)) { fpersistence::deserialize(jproperties, object.properties); }
    nlohmann::json jtexture;
    if(TRY_PARSE(nlohmann::json, j, "texture_asset", jtexture)) { fpersistence::deserialize(jtexture, object.texture_asset_ptr); }
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
    if(TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { fpersistence::deserialize(jorigin, object.origin); }
    nlohmann::json jscale;
    if(TRY_PARSE(nlohmann::json, j, "scale", jscale)) { fpersistence::deserialize(jscale, object.scale); }
    nlohmann::json jrotation;
    if(TRY_PARSE(nlohmann::json, j, "rotation", jrotation)) { fpersistence::deserialize(jrotation, object.rotation); }

    std::string name;
    if(TRY_PARSE(std::string, j, "custom_display_name", name)) { object.set_display_name(name); }
  }

  void vdeserialize_object::visit(hscene& object) const
  {
    nlohmann::json jambient_light_color;
    if(TRY_PARSE(nlohmann::json, j, "ambient_light_color", jambient_light_color)) { fpersistence::deserialize(jambient_light_color, object.ambient_light_color); }

    nlohmann::json jclear_color;
    if(TRY_PARSE(nlohmann::json, j, "clear_color", jclear_color)) { fpersistence::deserialize(jclear_color, object.clear_color); }

    nlohmann::json jrenderer;
    if(TRY_PARSE(nlohmann::json, j, "renderer", jrenderer))
    {
      std::string class_name;
      if(TRY_PARSE(std::string, jrenderer, "class_name", class_name)) // TODO move it to a helper function, every oobject can be serialized the same way - class_name + object
      {
        const oclass_object* class_o = REG.find_class(class_name);
        rrenderer_base* obj = REG.spawn_from_class<rrenderer_base>(class_o);
        if(obj != nullptr)
        {
          obj->accept(vdeserialize_object(jrenderer));
          object.renderer = obj;
        }
      }
    }

    nlohmann::json jcamera_conf;
    if(TRY_PARSE(nlohmann::json, j, "camera_config", jcamera_conf)) { fpersistence::deserialize(jcamera_conf, object.camera_config); }

    nlohmann::json jobjects;
    if(TRY_PARSE(nlohmann::json, j, "objects", jobjects))
    {
      for(const auto& jobj : jobjects)
      {
        std::string class_name;
        if(TRY_PARSE(std::string, jobj, "class_name", class_name))
        {
          const oclass_object* class_o = REG.find_class(class_name);
          hhittable_base* obj = REG.spawn_from_class<hhittable_base>(class_o);
          if(obj != nullptr)
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
    if(TRY_PARSE(nlohmann::json, j, "mesh_asset", jmesh)) { fpersistence::deserialize(jmesh, object.mesh_asset_ptr); }
    nlohmann::json jmaterial;
    if(TRY_PARSE(nlohmann::json, j, "material_asset", jmaterial)) { fpersistence::deserialize(jmaterial, object.material_asset_ptr); }
  }

  void vdeserialize_object::visit(hsphere& object) const
  {
    object.hhittable_base::accept(vdeserialize_object(j));
    TRY_PARSE(float, j, "radius", object.radius);
  }

  void vdeserialize_object::visit(hlight& object) const
  {
    // Only part of hittable
    nlohmann::json jorigin;
    if(TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { fpersistence::deserialize(jorigin, object.origin); }
    std::string name;
    if(TRY_PARSE(std::string, j, "custom_display_name", name)) { object.set_display_name(name); }

    nlohmann::json jproperties;
    if(TRY_PARSE(nlohmann::json, j, "properties", jproperties)) { fpersistence::deserialize(jproperties, object.properties); }
  }

  void vdeserialize_object::visit_rrenderer_base(rrenderer_base& object) const
  {
    TRY_PARSE(int, j, "output_width", object.output_width);
    TRY_PARSE(int, j, "output_height", object.output_height);
    nlohmann::json jmaterial;
    if(TRY_PARSE(nlohmann::json, j, "default_material_asset", jmaterial)) { fpersistence::deserialize(jmaterial, object.default_material_asset); }
  }

  void vdeserialize_object::visit(rgpu_forward_sync& object) const
  {
    visit_rrenderer_base(object);
    nlohmann::json jpixel_shader;
    if(TRY_PARSE(nlohmann::json, j, "pixel_shader_asset", jpixel_shader)) { fpersistence::deserialize(jpixel_shader, object.pixel_shader_asset); }
    nlohmann::json jvertex_shader;
    if(TRY_PARSE(nlohmann::json, j, "vertex_shader_asset", jvertex_shader)) { fpersistence::deserialize(jvertex_shader, object.vertex_shader_asset); }
  }

  void vdeserialize_object::visit(rgpu_deferred_sync& object) const
  {
    visit_rrenderer_base(object);
    nlohmann::json jgbuffer_pixel_shader;
    if(TRY_PARSE(nlohmann::json, j, "gbuffer_pixel_shader_asset", jgbuffer_pixel_shader)) { fpersistence::deserialize(jgbuffer_pixel_shader, object.gbuffer_pixel_shader_asset); }
    nlohmann::json jgbuffer_vertex_shader;
    if(TRY_PARSE(nlohmann::json, j, "gbuffer_vertex_shader_asset", jgbuffer_vertex_shader)) { fpersistence::deserialize(jgbuffer_vertex_shader, object.gbuffer_vertex_shader_asset); }
  }
}