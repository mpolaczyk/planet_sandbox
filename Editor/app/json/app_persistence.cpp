#include "stdafx.h"

#include <ppl.h>
#include <fstream>

#include "ui_persistence.h"
#include "app/app.h"
#include "hittables/scene.h"

#include "nlohmann/json.hpp"
#include "persistence/object_persistence.h"
#include "persistence/persistence.h"

namespace editor
{
  using namespace engine;

  void fapp_instance::load_scene_state()
  {
    LOG_INFO("Loading: scene");

    std::string path = fio::get_scene_file_path();
    std::ifstream input_stream(path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open file: {0}", path);
      return;
    }
    nlohmann::json j;
    input_stream >> j;

    nlohmann::json jscene_root;
    if (TRY_PARSE(nlohmann::json, j, "scene", jscene_root)) { scene_root->accept(vdeserialize_object(jscene_root)); }

    input_stream.close();
  }

  void fapp_instance::load_assets()
  {
    using namespace engine;

    LOG_INFO("Loading: textures");
    {
      std::vector<std::string> texture_names = fio::discover_texture_files(false);
      concurrency::parallel_for_each(begin(texture_names), end(texture_names),
        [&](const std::string& name)
        {
          atexture* temp = atexture::spawn();
          atexture::load(temp, name);
        });
    }
    
    LOG_INFO("Loading: materials");
    {
      std::vector<std::string> material_names = fio::discover_material_files(false);
      concurrency::parallel_for_each(begin(material_names), end(material_names),
        [&](const std::string& name)
        {
          amaterial* temp = amaterial::spawn();
          amaterial::load(temp, name);
        });
    }

    LOG_INFO("Loading: static meshes");
    {
      std::vector<std::string> mesh_names = fio::discover_mesh_files(false);
      concurrency::parallel_for_each(begin(mesh_names), end(mesh_names),
        [&](const std::string& name)
        {
          astatic_mesh* temp = astatic_mesh::spawn();
          astatic_mesh::load(temp, name);
        });
    }
  }

  void fapp_instance::load_window_state()
  {
    LOG_INFO("Loading: window state");

    std::string work_file = fio::get_window_file_path();
    std::ifstream input_stream(work_file.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open file: {0}", work_file);
      return;
    }
    nlohmann::json j;
    input_stream >> j;

    nlohmann::json jwindow;
    if (TRY_PARSE(nlohmann::json, j, "window", jwindow)) { fui_persistence::deserialize(jwindow, window_conf); }

    TRY_PARSE(float, j, "zoom", output_window_model.zoom);

    input_stream.close();
  }


  void fapp_instance::save_scene_state()
  {
    LOG_INFO("Saving: scene");

    nlohmann::json j;
    scene_root->accept(vserialize_object(j["scene"]));
    std::ofstream o(fio::get_scene_file_path().c_str(), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    o.close();
  }

  void fapp_instance::save_materials()
  {
    LOG_INFO("Saving: materials");

    std::vector<amaterial*> materials = REG.get_all_by_type<amaterial>();
    for (amaterial* m : materials)
    {
      amaterial::save(m);
    }
  }

  void fapp_instance::save_window_state()
  {
    LOG_INFO("Saving: window state");

    nlohmann::json j;
    j["window"] = fui_persistence::serialize(window_conf);
    j["zoom"] = output_window_model.zoom;
    std::ofstream o(fio::get_window_file_path().c_str(), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    o.close();
  }
}