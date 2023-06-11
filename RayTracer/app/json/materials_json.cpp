#include "stdafx.h"

#include "app/factories.h"

#include "app/json/vec3_json.h"
#include "app/json/materials_json.h"

nlohmann::json material_instances_serializer::serialize(const asset_registry<material>* value)
{
  assert(value != nullptr);
  nlohmann::json jarr = nlohmann::json::array();
  for (const std::pair<std::string, material*> pair : value->registry)
  {
    jarr.push_back(material_serializer::serialize(pair.second));
  }
  return jarr;
}

nlohmann::json material_serializer::serialize(const material* value)
{
  assert(value != nullptr);
  nlohmann::json j;
  j["color"] = vec3_serializer::serialize(value->color);
  j["emitted_color"] = vec3_serializer::serialize(value->emitted_color);
  j["gloss_color"] = vec3_serializer::serialize(value->gloss_color);
  j["type"] = value->type;
  j["id"] = value->id;
  j["smoothness"] = value->smoothness;
  j["gloss_probability"] = value->gloss_probability;
  j["refraction_probability"] = value->refraction_probability;
  j["refraction_index"] = value->refraction_index;
  return j;
}


void material_instances_serializer::deserialize(const nlohmann::json& j, asset_registry<material>* out_value)
{
  assert(out_value != nullptr);
  for (const auto& element : j)
  {
    material* obj = object_factory::spawn_material(element["type"]);
    material_serializer::deserialize(element, obj);
    out_value->try_add(obj);
  }
}

void material_serializer::deserialize(const nlohmann::json& j, material* out_value)
{
  assert(out_value != nullptr);

  TRY_PARSE(material_type, j, "type", out_value->type);
  TRY_PARSE(std::string, j, "id", out_value->id);

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