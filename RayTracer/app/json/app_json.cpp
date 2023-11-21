#include "stdafx.h"

#include <fstream>

#include "app/json/app_json.h"
#include "persistence/vec3_json.h"
#include "persistence/frame_renderer_json.h"
#include "persistence/assets_json.h"
#include "persistence/hittables_json.h"

#include "math/camera.h"

#include "nlohmann/json.hpp"
#include "persistence/camera_config_json.h"

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

void app_instance::load_scene_state()
{
  LOG_INFO("Loading: scene");

  std::string path = engine::io::get_scene_file_path();
  std::ifstream input_stream(path.c_str());
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
    engine::material_asset* temp = engine::material_asset::load(name);
  }

  LOG_INFO("Loading: textures");
  std::vector<std::string> texture_names = engine::io::discover_texture_files(false);
  for (const std::string& name : texture_names)
  {
    texture_asset* temp = texture_asset::load(name);
  }

  LOG_INFO("Loading: static meshes");
  std::vector<std::string> mesh_names = engine::io::discover_mesh_files(false);
  for (const std::string& name : mesh_names)
  {
    engine::static_mesh_asset* temp = engine::static_mesh_asset::load(name);
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

  std::vector<engine::material_asset*> materials = engine::get_object_registry()->get_all_by_type<engine::material_asset>();
  for (engine::material_asset* m : materials)
  {
    engine::material_asset::save(m);
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
