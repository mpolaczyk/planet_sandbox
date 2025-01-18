#include "stdafx.h"

#include <cassert>

#include "engine/persistence/object_persistence.h"
#include "engine/persistence/persistence_helper.h"
#include "engine/persistence/persistence.h"

#include "assets/material.h"
#include "assets/mesh.h"
#include "assets/texture.h"
#include "assets/vertex_shader.h"
#include "assets/pixel_shader.h"

#include "engine/hittable.h"
#include "hittables/static_mesh.h"
#include "hittables/scene.h"

#include "engine/renderer/renderer_base.h"
#include "renderers/forward.h"
#include "renderers/deferred.h"
#include "renderers/ray_tracing.h"

#include "hittables/light.h"
#include "core/rtti/object_registry.h"

namespace engine
{
  template <typename T>
  bool try_parse(const nlohmann::json& j, const std::string& key, T& out_value, const char* function_name)
  {
    if (j.contains(key))
    {
      out_value = j[key];
      return true;
    }
    if (function_name != nullptr)
    {
      LOG_WARN("json try_parse key missing: {0} in function {1}", key, function_name);
    }
    else
    {
      LOG_WARN("json try_parse key missing: {0}", key);
    }
    return false;
  }

  template bool ENGINE_API try_parse<bool>(const nlohmann::json& j, const std::string& key, bool& out_value, const char* function_name);
  template bool ENGINE_API try_parse<int>(const nlohmann::json& j, const std::string& key, int& out_value, const char* function_name);
  template bool ENGINE_API try_parse<float>(const nlohmann::json& j, const std::string& key, float& out_value, const char* function_name);
  template bool ENGINE_API try_parse<int32_t>(const nlohmann::json& j, const std::string& key, int32_t& out_value, const char* function_name);

  template bool ENGINE_API try_parse<std::string>(const nlohmann::json& j, const std::string& key, std::string& out_value, const char* function_name);
  template bool ENGINE_API try_parse<nlohmann::json>(const nlohmann::json& j, const std::string& key, nlohmann::json& out_value, const char* function_name);


  nlohmann::json fpersistence::serialize(const fvec3& value)
  {
    nlohmann::json j;
    j["x"] = value.x;
    j["y"] = value.y;
    j["z"] = value.z;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, fvec3& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.x);
    TRY_PARSE(float, j, "y", out_value.y);
    TRY_PARSE(float, j, "z", out_value.z);
  }

  nlohmann::json fpersistence::serialize(const DirectX::XMFLOAT3& value)
  {
    nlohmann::json j;
    j["x"] = value.x;
    j["y"] = value.y;
    j["z"] = value.z;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, DirectX::XMFLOAT3& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.x);
    TRY_PARSE(float, j, "y", out_value.y);
    TRY_PARSE(float, j, "z", out_value.z);
  }

  nlohmann::json fpersistence::serialize(const DirectX::XMFLOAT4& value)
  {
    nlohmann::json j;
    j["x"] = value.x;
    j["y"] = value.y;
    j["z"] = value.z;
    j["w"] = value.w;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, DirectX::XMFLOAT4& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.x);
    TRY_PARSE(float, j, "y", out_value.y);
    TRY_PARSE(float, j, "z", out_value.z);
    TRY_PARSE(float, j, "w", out_value.w);
  }

  nlohmann::json fpersistence::serialize(const DirectX::XMVECTORF32& value)
  {
    nlohmann::json j;
    j["x"] = value.f[0];
    j["y"] = value.f[1];
    j["z"] = value.f[2];
    j["w"] = value.f[3];
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, DirectX::XMVECTORF32& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.f[0]);
    TRY_PARSE(float, j, "y", out_value.f[1]);
    TRY_PARSE(float, j, "z", out_value.f[2]);
    TRY_PARSE(float, j, "w", out_value.f[3]);
  }

  nlohmann::json fpersistence::serialize(const fsoft_asset_ptr_base& value)
  {
    nlohmann::json j;
    j["name"] = value.name;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, fsoft_asset_ptr_base& out_value)
  {
    TRY_PARSE(std::string, j, "name", out_value.name);
  }

  nlohmann::json fpersistence::serialize(const fcamera& value)
  {
    nlohmann::json j;
    j["field_of_view"] = value.field_of_view;
    j["look_from"] = fpersistence::serialize(value.location);
    j["pitch"] = value.pitch;
    j["yaw"] = value.yaw;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, fcamera& out_value)
  {
    TRY_PARSE(float, j, "field_of_view", out_value.field_of_view);
    TRY_PARSE(float, j, "pitch", out_value.pitch);
    TRY_PARSE(float, j, "yaw", out_value.yaw);

    nlohmann::json jlook_from;
    if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { fpersistence::deserialize(jlook_from, out_value.location); }
  }

  nlohmann::json fpersistence::serialize(const flight_properties& value)
  {
    nlohmann::json j;
    j["direction"] = fpersistence::serialize(value.direction);
    j["color"] = fpersistence::serialize(value.color);
    j["spot_angle"] = value.spot_angle;
    j["constant_attenuation"] = value.constant_attenuation;
    j["linear_attenuation"] = value.linear_attenuation;
    j["quadratic_attenuation"] = value.quadratic_attenuation;
    j["light_type"] = value.light_type;
    j["enabled"] = value.enabled;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, flight_properties& out_value)
  {
    nlohmann::json jdirection;
    if (TRY_PARSE(nlohmann::json, j, "direction", jdirection)) { fpersistence::deserialize(jdirection, out_value.direction); }

    nlohmann::json jcolor;
    if (TRY_PARSE(nlohmann::json, j, "color", jcolor)) { fpersistence::deserialize(jcolor, out_value.color); }

    TRY_PARSE(float, j, "spot_angle", out_value.spot_angle);
    TRY_PARSE(float, j, "constant_attenuation", out_value.constant_attenuation);
    TRY_PARSE(float, j, "linear_attenuation", out_value.linear_attenuation);
    TRY_PARSE(float, j, "quadratic_attenuation", out_value.quadratic_attenuation);
    TRY_PARSE(int, j, "light_type", out_value.light_type);
    TRY_PARSE(int, j, "enabled", out_value.enabled);
  }

  nlohmann::json fpersistence::serialize(const fmaterial_properties& value)
  {
    nlohmann::json j;
    j["emissive"] = fpersistence::serialize(value.emissive);
    j["ambient"] = fpersistence::serialize(value.ambient);
    j["diffuse"] = fpersistence::serialize(value.diffuse);
    j["specular"] = fpersistence::serialize(value.specular);
    j["specular_power"] = value.specular_power;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, fmaterial_properties& out_value)
  {
    nlohmann::json jemissive;
    if (TRY_PARSE(nlohmann::json, j, "emissive", jemissive)) { fpersistence::deserialize(jemissive, out_value.emissive); }
    nlohmann::json jambient;
    if (TRY_PARSE(nlohmann::json, j, "ambient", jambient)) { fpersistence::deserialize(jambient, out_value.ambient); }
    nlohmann::json jdiffuse;
    if (TRY_PARSE(nlohmann::json, j, "diffuse", jdiffuse)) { fpersistence::deserialize(jdiffuse, out_value.diffuse); }
    nlohmann::json jspecular;
    if (TRY_PARSE(nlohmann::json, j, "specular", jspecular)) { fpersistence::deserialize(jspecular, out_value.specular); }
    TRY_PARSE(float, j, "specular_power", out_value.specular_power);
  }



  void vserialize_object::visit(amaterial& object) const
  {
    j["properties"] = fpersistence::serialize(object.properties);
    j["texture_asset"] = fpersistence::serialize(object.texture_asset_ptr);
  }

  void vserialize_object::visit(atexture& object) const
  {
    j["img_file_name"] = object.img_file_name;
  }

  void vserialize_object::visit(astatic_mesh& object) const
  {
    j["obj_file_name"] = object.obj_file_name;
  }

  void vserialize_object::visit(ashader& object) const
  {
    j["shader_file_name"] = object.shader_file_name;
    j["entrypoint"] = object.entrypoint;
    j["target"] = object.target;
    j["cache_file_name"] = object.cache_file_name;
  }

  void vserialize_object::visit(hhittable_base& object) const
  {
    j["origin"] = fpersistence::serialize(object.origin);
    j["scale"] = fpersistence::serialize(object.scale);
    j["rotation"] = fpersistence::serialize(object.rotation);
    j["gravity_enabled"] = object.gravity_enabled;
    j["rigid_body_type"] = object.rigid_body_type;
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
    j["camera_config"] = fpersistence::serialize(object.camera);
  }

  void vserialize_object::visit(hstatic_mesh& object) const
  {
    object.hhittable_base::accept(vserialize_object(j));
    j["mesh_asset"] = fpersistence::serialize(object.mesh_asset_ptr);
    j["material_asset"] = fpersistence::serialize(object.material_asset_ptr);
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

  }

  void vserialize_object::visit(rforward& object) const
  {
    visit_rrenderer_base(object);
  }

  void vserialize_object::visit(rdeferred& object) const
  {
    visit_rrenderer_base(object);
  }

  void vserialize_object::visit(rray_tracing& object) const
  {
    // TODO
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
    TRY_PARSE(std::string, j, "img_file_name", object.img_file_name);
  }

  void vdeserialize_object::visit(astatic_mesh& object) const
  {
    TRY_PARSE(std::string, j, "obj_file_name", object.obj_file_name);
  }

  void vdeserialize_object::visit(ashader& object) const
  {
    TRY_PARSE(std::string, j, "shader_file_name", object.shader_file_name);
    TRY_PARSE(std::string, j, "entrypoint", object.entrypoint);
    TRY_PARSE(std::string, j, "target", object.target);
    TRY_PARSE(std::string, j, "cache_file_name", object.cache_file_name);
  }
  
  void vdeserialize_object::visit(hhittable_base& object) const
  {
    nlohmann::json jorigin;
    if(TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { fpersistence::deserialize(jorigin, object.origin); }
    nlohmann::json jscale;
    if(TRY_PARSE(nlohmann::json, j, "scale", jscale)) { fpersistence::deserialize(jscale, object.scale); }
    nlohmann::json jrotation;
    if(TRY_PARSE(nlohmann::json, j, "rotation", jrotation)) { fpersistence::deserialize(jrotation, object.rotation); }

    TRY_PARSE(bool, j, "gravity_enabled", object.gravity_enabled);
    TRY_PARSE(int, j, "rigid_body_type", object.rigid_body_type);

    std::string name;
    if(TRY_PARSE(std::string, j, "custom_display_name", name)) { object.set_display_name(name); }
  }

  void vdeserialize_object::visit(hscene& object) const
  {
    nlohmann::json jambient_light_color;
    if(TRY_PARSE(nlohmann::json, j, "ambient_light_color", jambient_light_color)) { fpersistence::deserialize(jambient_light_color, object.ambient_light_color); }

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
    if(TRY_PARSE(nlohmann::json, j, "camera_config", jcamera_conf)) { fpersistence::deserialize(jcamera_conf, object.camera); }

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

  }

  void vdeserialize_object::visit(rforward& object) const
  {
    visit_rrenderer_base(object);
  }

  void vdeserialize_object::visit(rdeferred& object) const
  {
    visit_rrenderer_base(object);
  }
  
  void vdeserialize_object::visit(rray_tracing& object) const
  {
    // TODO
  }
}