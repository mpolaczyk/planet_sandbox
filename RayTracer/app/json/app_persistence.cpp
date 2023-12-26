#include "stdafx.h"

#include <fstream>

#include "ui_persistence.h"
#include "app/app.h"
#include "hittables/scene.h"

#include "nlohmann/json.hpp"
#include "persistence/object_persistence.h"
#include "persistence/persistence.h"

namespace ray_tracer
{
  using namespace engine;

  void app_instance::load_scene_state()
  {
    LOG_INFO("Loading: scene");

    std::string path = io::get_scene_file_path();
    std::ifstream input_stream(path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open file: {0}", path);
      return;
    }
    nlohmann::json j;
    input_stream >> j;

    nlohmann::json jcamera_conf;
    if (TRY_PARSE(nlohmann::json, j, "camera_config", jcamera_conf)) { persistence::deserialize(jcamera_conf, camera_conf); }
   
    nlohmann::json jscene_root;
    if (TRY_PARSE(nlohmann::json, j, "scene", jscene_root)) { scene_root->accept(deserialize_object(jscene_root)); }

    input_stream.close();
  }

  void app_instance::load_rendering_state()
  {
    LOG_INFO("Loading: rendering state");

    std::string rendering_file = io::get_rendering_file_path();
    std::ifstream input_stream(rendering_file.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open file: {0}", rendering_file);
      return;
    }
    nlohmann::json j;
    input_stream >> j;
    nlohmann::json jrenderer_conf;
    if (TRY_PARSE(nlohmann::json, j, "renderer_config", jrenderer_conf)) { persistence::deserialize(jrenderer_conf, renderer_conf); }
    input_stream.close();  
  }

  void app_instance::load_assets()
  {
    using namespace engine;
    LOG_INFO("Loading: materials");
    std::vector<std::string> material_names = io::discover_material_files(false);
    for (const std::string& name : material_names)
    {
      material_asset* temp = material_asset::spawn();
      material_asset::load(temp, name);
    }

    LOG_INFO("Loading: textures");
    std::vector<std::string> texture_names = io::discover_texture_files(false);
    for (const std::string& name : texture_names)
    {
      texture_asset* temp = texture_asset::spawn();
      texture_asset::load(temp, name);
    }

    LOG_INFO("Loading: static meshes");
    std::vector<std::string> mesh_names = io::discover_mesh_files(false);
    for (const std::string& name : mesh_names)
    {
      static_mesh_asset* temp = static_mesh_asset::spawn();
      static_mesh_asset::load(temp, name);
    }
  }

  void app_instance::load_window_state()
  {
    LOG_INFO("Loading: window state");

    std::string work_file = io::get_window_file_path();
    std::ifstream input_stream(work_file.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open file: {0}", work_file);
      return;
    }
    nlohmann::json j;
    input_stream >> j;

    nlohmann::json jwindow;
    if (TRY_PARSE(nlohmann::json, j, "window", jwindow)) { ui_persistence::deserialize(jwindow, window_conf); }

    TRY_PARSE(bool, j, "auto_render", ow_model.auto_render);
    TRY_PARSE(float, j, "zoom", ow_model.zoom);
  
    input_stream.close();
  }


  void app_instance::save_scene_state()
  {
    LOG_INFO("Saving: scene");

    nlohmann::json j;
    j["camera_config"] = persistence::serialize(camera_conf);
    scene_root->accept(serialize_object(j["scene"]));
    std::ofstream o(io::get_scene_file_path().c_str(), std::ios_base::out | std::ios::binary);
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
    j["renderer_config"] = persistence::serialize(renderer_conf);
    std::ofstream o(io::get_rendering_file_path().c_str(), std::ios_base::out | std::ios::binary);
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

    std::vector<material_asset*> materials = REG.get_all_by_type<material_asset>();
    for (material_asset* m : materials)
    {
      material_asset::save(m);
    }
  }

  void app_instance::save_window_state()
  {
    LOG_INFO("Saving: window state");

    nlohmann::json j;
    j["window"] = ui_persistence::serialize(window_conf);
    j["auto_render"] = ow_model.auto_render;
    j["zoom"] = ow_model.zoom;
    std::ofstream o(io::get_window_file_path().c_str(), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    o.close();
  }
}