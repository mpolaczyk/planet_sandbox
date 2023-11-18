#include "stdafx.h"

#include <fstream>

#include "app/json/app_json.h"
#include "persistence/vec3_json.h"
#include "app/json/frame_renderer_json.h"
#include "persistence/assets_json.h"
#include "app/json/hittables_json.h"

#include "math/camera.h"

nlohmann::json window_config_serializer::serialize(const window_config& value)
{
  nlohmann::json j;
  j["x"] = value.x;
  j["y"] = value.y;
  j["w"] = value.w;
  j["h"] = value.h;
  return j;
}

window_config window_config_serializer::deserialize(const nlohmann::json& j)
{
  window_config value;
  TRY_PARSE(int, j, "x", value.x);
  TRY_PARSE(int, j, "y", value.y);
  TRY_PARSE(int, j, "w", value.w);
  TRY_PARSE(int, j, "h", value.h);
  return value;
}

nlohmann::json camera_config_serializer::serialize(const camera_config& value)
{
  nlohmann::json j;
  j["field_of_view"] = value.field_of_view;
  j["aspect_ratio_h"] = value.aspect_ratio_h;
  j["aspect_ratio_w"] = value.aspect_ratio_w;
  j["aperture"] = value.aperture;
  j["dist_to_focus"] = value.dist_to_focus;
  j["type"] = value.type;
  j["look_from"] = vec3_serializer::serialize(value.look_from);
  j["look_dir"] = vec3_serializer::serialize(value.look_dir);
  return j;
}

camera_config camera_config_serializer::deserialize(const nlohmann::json& j)
{
  camera_config value;
  TRY_PARSE(float, j, "field_of_view", value.field_of_view);
  TRY_PARSE(float, j, "aspect_ratio_h", value.aspect_ratio_h);
  TRY_PARSE(float, j, "aspect_ratio_w", value.aspect_ratio_w);
  TRY_PARSE(float, j, "aperture", value.aperture);
  TRY_PARSE(float, j, "dist_to_focus", value.dist_to_focus);
  TRY_PARSE(float, j, "type", value.type);

  nlohmann::json jlook_dir;
  if (TRY_PARSE(nlohmann::json, j, "look_dir", jlook_dir)) { value.look_dir = vec3_serializer::deserialize(jlook_dir); }

  nlohmann::json jlook_from;
  if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { value.look_from = vec3_serializer::deserialize(jlook_from); }

  return value;
}

void app_instance::load_scene_state()
{
  LOG_INFO("Loading: scene");

  std::ifstream input_stream(engine::io::get_scene_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jcamera_conf;
  if (TRY_PARSE(nlohmann::json, j, "camera_config", jcamera_conf)) { *camera_conf = camera_config_serializer::deserialize(jcamera_conf); }

  nlohmann::json jscene_root;
  if (TRY_PARSE(nlohmann::json, j, "scene", jscene_root)) { scene_serializer::deserialize(jscene_root, scene_root); }

  input_stream.close();
}

void app_instance::load_rendering_state()
{
  LOG_INFO("Loading: rendering state");

  std::ifstream input_stream(engine::io::get_rendering_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;
  nlohmann::json jrenderer_conf;
  if (TRY_PARSE(nlohmann::json, j, "renderer_config", jrenderer_conf)) { *renderer_conf = renderer_config_serializer::deserialize(jrenderer_conf); }
  input_stream.close();  
}

void app_instance::load_assets()
{
  LOG_INFO("Loading: materials");
  std::vector<std::string> material_names = engine::io::discover_material_files(false);
  for (const std::string& name : material_names)
  {
    engine::material* temp = engine::material::load(name);
    if (temp != nullptr)
    {
      engine::get_asset_registry()->add<engine::material>(temp, name);
    }
  }

  LOG_INFO("Loading: textures");
  std::vector<std::string> texture_names = engine::io::discover_texture_files(false);
  for (const std::string& name : texture_names)
  {
    texture* temp = texture::load(name);
    if (temp != nullptr)
    {
      engine::get_asset_registry()->add<texture>(temp, name);
    }
  }

  LOG_INFO("Loading: static meshes");
  std::vector<std::string> mesh_names = engine::io::discover_mesh_files(false);
  for (const std::string& name : mesh_names)
  {
    engine::mesh* temp = engine::mesh::load(name);
    if (temp != nullptr)
    {
      engine::get_asset_registry()->add<engine::mesh>(temp, name);
    }
  }

}

void app_instance::load_window_state()
{
  LOG_INFO("Loading: window state");

  std::ifstream input_stream(engine::io::get_window_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jwindow;
  if (TRY_PARSE(nlohmann::json, j, "window", jwindow)) { window_conf = window_config_serializer::deserialize(jwindow); }

  TRY_PARSE(bool, j, "auto_render", ow_model.auto_render);
  TRY_PARSE(float, j, "zoom", ow_model.zoom);
  
  input_stream.close();
}


void app_instance::save_scene_state()
{
  LOG_INFO("Saving: scene");

  nlohmann::json j;
  j["camera_config"] = camera_config_serializer::serialize(*camera_conf);
  j["scene"] = scene_serializer::serialize(scene_root);
  std::ofstream o(engine::io::get_scene_file_path().c_str(), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}

void app_instance::save_rendering_state()
{
  LOG_INFO("Saving: rendering state");

  nlohmann::json j;
  j["renderer_config"] = renderer_config_serializer::serialize(*renderer_conf);
  std::ofstream o(engine::io::get_rendering_file_path().c_str(), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}

void app_instance::save_materials()
{
  LOG_INFO("Saving: materials");

  std::vector<engine::material*> materials = engine::get_asset_registry()->get_assets<engine::material>();
  for (engine::material* m : materials)
  {
    engine::material::save(m);
  }
}

void app_instance::save_window_state()
{
  LOG_INFO("Saving: window state");

  nlohmann::json j;
  j["window"] = window_config_serializer::serialize(window_conf);
  j["auto_render"] = ow_model.auto_render;
  j["zoom"] = ow_model.zoom;
  std::ofstream o(engine::io::get_window_file_path().c_str(), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}
