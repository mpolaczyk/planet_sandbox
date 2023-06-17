#include "stdafx.h"

#include "math/hittables.h"

#include "app/json/vec3_json.h"
#include "app/json/hittables_json.h"
#include "app/json/assets_json.h"


nlohmann::json hittable_serializer::serialize(const hittable* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j["name"] = value->id;  // TODO do we need this? also name is misleading
  j["type"] = value->type;
  j["material_asset"] = soft_asset_ptr_base_serializer::serialize(&value->material_asset);
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

nlohmann::json scene_serializer::serialize(const scene* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j = hittable_serializer::serialize(value);
  nlohmann::json jarr = nlohmann::json::array();
  for (const hittable* object : value->objects)
  {
    switch (object->type)
    {
    case hittable_type::scene:
      jarr.push_back(scene_serializer::serialize(static_cast<const scene*>(object)));
      break;
    case hittable_type::sphere:
      jarr.push_back(sphere_serializer::serialize(static_cast<const sphere*>(object)));
      break;
    case hittable_type::xy_rect:
      jarr.push_back(xy_rect_serializer::serialize(static_cast<const xy_rect*>(object)));
      break;
    case hittable_type::xz_rect:
      jarr.push_back(xz_rect_serializer::serialize(static_cast<const xz_rect*>(object)));
      break;
    case hittable_type::yz_rect:
      jarr.push_back(yz_rect_serializer::serialize(static_cast<const yz_rect*>(object)));
      break;
    case hittable_type::static_mesh:
      jarr.push_back(static_mesh_serializer::serialize(static_cast<const static_mesh*>(object)));
      break;
    }
  }
  j["objects"] = jarr;
  return j;
}

nlohmann::json xy_rect_serializer::serialize(const xy_rect* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j = hittable_serializer::serialize(value);
  j["x0"] = value->x0;
  j["y0"] = value->y0;
  j["x1"] = value->x1;
  j["y1"] = value->y1;
  j["z"] = value->z;
  return j;
}

nlohmann::json xz_rect_serializer::serialize(const xz_rect* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j = hittable_serializer::serialize(value);
  j["x0"] = value->x0;
  j["z0"] = value->z0;
  j["x1"] = value->x1;
  j["z1"] = value->z1;
  j["y"] = value->y;
  return j;
}

nlohmann::json yz_rect_serializer::serialize(const yz_rect* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j = hittable_serializer::serialize(value);
  j["y0"] = value->y0;
  j["z0"] = value->z0;
  j["y1"] = value->y1;
  j["z1"] = value->z1;
  j["x"] = value->x;
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
  j["mesh_asset"] = soft_asset_ptr_base_serializer::serialize(&value->mesh_asset);
  return j;
}


void hittable_serializer::deserialize(const nlohmann::json& j, hittable* out_value)
{
  assert(out_value != nullptr);
  TRY_PARSE(int, j, "name", out_value->id);
  TRY_PARSE(hittable_type, j, "type", out_value->type);
  
  nlohmann::json jmaterial;
  if (TRY_PARSE(nlohmann::json, j, "material_asset", jmaterial)) { soft_asset_ptr_base_serializer::deserialize(jmaterial, &out_value->material_asset); }
}

void sphere_serializer::deserialize(const nlohmann::json& j, sphere* out_value)
{
  assert(out_value != nullptr);
  hittable_serializer::deserialize(j, out_value);
  
  TRY_PARSE(float, j, "radius", out_value->radius);

  nlohmann::json jorigin;
  if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { out_value->origin = vec3_serializer::deserialize(jorigin); }
}

void scene_serializer::deserialize(const nlohmann::json& j, scene* out_value)
{
  assert(out_value != nullptr);

  nlohmann::json jobjects;
  if (TRY_PARSE(nlohmann::json, j, "objects", jobjects))
  {
    for (const auto& jobj : jobjects)
    {
      hittable_type type;
      if (TRY_PARSE(hittable_type, jobj, "type", type))
      {
        hittable* obj = object_factory::spawn_hittable(type);
        switch (obj->type)
        {
        case hittable_type::scene:
          scene_serializer::deserialize(jobj, static_cast<scene*>(obj));
          break;
        case hittable_type::sphere:
          sphere_serializer::deserialize(jobj, static_cast<sphere*>(obj));
          break;
        case hittable_type::xy_rect:
          xy_rect_serializer::deserialize(jobj, static_cast<xy_rect*>(obj));
          break;
        case hittable_type::xz_rect:
          xz_rect_serializer::deserialize(jobj, static_cast<xz_rect*>(obj));
          break;
        case hittable_type::yz_rect:
          yz_rect_serializer::deserialize(jobj, static_cast<yz_rect*>(obj));
          break;
        case hittable_type::static_mesh:
          static_mesh_serializer::deserialize(jobj, static_cast<static_mesh*>(obj));
          break;
        }
        out_value->objects.push_back(obj);
      }
    }
  }
}

void xy_rect_serializer::deserialize(const nlohmann::json& j, xy_rect* out_value)
{
  assert(out_value != nullptr);
  hittable_serializer::deserialize(j, out_value);
  
  TRY_PARSE(float, j, "x0", out_value->x0);
  TRY_PARSE(float, j, "y0", out_value->y0);
  TRY_PARSE(float, j, "x1", out_value->x1);
  TRY_PARSE(float, j, "y1", out_value->y1);
  TRY_PARSE(float, j, "z", out_value->z);
}

void xz_rect_serializer::deserialize(const nlohmann::json& j, xz_rect* out_value)
{
  assert(out_value != nullptr);
  hittable_serializer::deserialize(j, out_value);

  TRY_PARSE(float, j, "x0", out_value->x0);
  TRY_PARSE(float, j, "z0", out_value->z0);
  TRY_PARSE(float, j, "x1", out_value->x1);
  TRY_PARSE(float, j, "z1", out_value->z1);
  TRY_PARSE(float, j, "y", out_value->y);
}

void yz_rect_serializer::deserialize(const nlohmann::json& j, yz_rect* out_value)
{
  assert(out_value != nullptr);
  hittable_serializer::deserialize(j, out_value);
  
  TRY_PARSE(float, j, "y0", out_value->y0);
  TRY_PARSE(float, j, "z0", out_value->z0);
  TRY_PARSE(float, j, "y1", out_value->y1);
  TRY_PARSE(float, j, "z1", out_value->z1);
  TRY_PARSE(float, j, "x", out_value->x);
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
  if (TRY_PARSE(nlohmann::json, j, "mesh_asset", jmesh)) { soft_asset_ptr_base_serializer::deserialize(jmesh, &out_value->mesh_asset); }
}