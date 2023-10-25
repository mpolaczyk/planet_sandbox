#include "stdafx.h"

#include "app/factories.h"
#include "app/asset.h"

#include "app/json/vec3_json.h"
#include "app/json/assets_json.h"

#include "math/materials.h"
#include "math/mesh.h"
#include "math/textures.h"

#include "math/colors.h"

nlohmann::json material_serializer::serialize(const material* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j["color"] = vec3_serializer::serialize(value->color);
  j["emitted_color"] = vec3_serializer::serialize(value->emitted_color);
  j["gloss_color"] = vec3_serializer::serialize(value->gloss_color);
  j["type"] = value->type;
  j["smoothness"] = value->smoothness;
  j["gloss_probability"] = value->gloss_probability;
  j["refraction_probability"] = value->refraction_probability;
  j["refraction_index"] = value->refraction_index;
  return j;
}

void material_serializer::deserialize(const nlohmann::json& j, material* out_value)
{
  assert(out_value != nullptr);

  TRY_PARSE(material_type, j, "type", out_value->type);
  
  nlohmann::json jcolor;
  if (TRY_PARSE(nlohmann::json, j, "color", jcolor)) { out_value->color = vec3_serializer::deserialize(jcolor); }
  assert(colors::is_valid(out_value->color));

  nlohmann::json jemitted_color;
  if (TRY_PARSE(nlohmann::json, j, "emitted_color", jemitted_color)) { out_value->emitted_color = vec3_serializer::deserialize(jemitted_color); }
  assert(colors::is_valid(out_value->emitted_color));

  TRY_PARSE(float, j, "smoothness", out_value->smoothness);
  assert(out_value->smoothness >= 0.0f && out_value->smoothness <= 1.0f);

  TRY_PARSE(float, j, "gloss_probability", out_value->gloss_probability);
  assert(out_value->gloss_probability >= 0.0f && out_value->gloss_probability <= 1.0f);

  nlohmann::json jgloss_color;
  if (TRY_PARSE(nlohmann::json, j, "gloss_color", jgloss_color)) { out_value->gloss_color = vec3_serializer::deserialize(jgloss_color); }
  assert(colors::is_valid(out_value->gloss_color));

  TRY_PARSE(float, j, "refraction_probability", out_value->refraction_probability);
  assert(out_value->refraction_probability >= 0.0f && out_value->refraction_probability <= 1.0f);
  TRY_PARSE(float, j, "refraction_index", out_value->refraction_index);
}



nlohmann::json mesh_serializer::serialize(const mesh* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j["shape_index"] = value->shape_index;
  j["obj_file_name"] = value->obj_file_name;
  return j;
}

void mesh_serializer::deserialize(const nlohmann::json& j, mesh* out_value)
{
  assert(out_value != nullptr);

  TRY_PARSE(int, j, "shape_index", out_value->shape_index);

  TRY_PARSE(std::string, j, "obj_file_name", out_value->obj_file_name);
}



nlohmann::json texture_serializer::serialize(const texture* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j["width"] = value->width;
  j["height"] = value->height;
  j["img_file_name"] = value->img_file_name;
  return j;
}

void texture_serializer::deserialize(const nlohmann::json& j, texture* out_value)
{
  assert(out_value != nullptr);

  TRY_PARSE(int, j, "width", out_value->width);
  TRY_PARSE(int, j, "height", out_value->height);

  TRY_PARSE(std::string, j, "img_file_name", out_value->img_file_name);
}



nlohmann::json soft_asset_ptr_base_serializer::serialize(const soft_asset_ptr_base* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j["name"] = value->name;
  return j;
}

void soft_asset_ptr_base_serializer::deserialize(const nlohmann::json& j, soft_asset_ptr_base* out_value)
{
  assert(out_value != nullptr);

  TRY_PARSE(std::string, j, "name", out_value->name);
}