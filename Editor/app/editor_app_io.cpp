#include "stdafx.h"

#include "app/editor_app.h"

namespace editor
{
  void feditor_app::save_materials() 
  {
    LOG_INFO("Saving: materials");

    std::vector<amaterial*> materials = REG.get_all_by_type<amaterial>();
    for (amaterial* m : materials)
    {
      m->save();
    }
  }

  void feditor_app::load_assets() 
  {
    LOG_INFO("Loading: textures");
    {
      std::vector<std::string> texture_names = fio::discover_texture_files(false);
      concurrency::parallel_for_each(begin(texture_names), end(texture_names),
        [&](const std::string& name)
        {
          atexture* temp = atexture::spawn();
          temp->load(name);
        });
    }
    
    LOG_INFO("Loading: materials");
    {
      std::vector<std::string> material_names = fio::discover_material_files(false);
      concurrency::parallel_for_each(begin(material_names), end(material_names),
        [&](const std::string& name)
        {
          amaterial* temp = amaterial::spawn();
          temp->load(name);
        });
    }

    LOG_INFO("Loading: static meshes");
    {
      std::vector<std::string> mesh_names = fio::discover_mesh_files(false);
      concurrency::parallel_for_each(begin(mesh_names), end(mesh_names),
        [&](const std::string& name)
        {
          astatic_mesh* temp = astatic_mesh::spawn();
          temp->load(name);
        });
    }

    LOG_INFO("Loading: pixel shaders");
    {
      std::vector<std::string> shader_names = fio::discover_pixel_shader_files(false);
      concurrency::parallel_for_each(begin(shader_names), end(shader_names),
        [&](const std::string& name)
        {
          apixel_shader* temp = apixel_shader::spawn();
          temp->load(name);
        });
    }

    LOG_INFO("Loading: vertex shaders");
    {
      std::vector<std::string> shader_names = fio::discover_vertex_shader_files(false);
      concurrency::parallel_for_each(begin(shader_names), end(shader_names),
        [&](const std::string& name)
        {
          avertex_shader* temp = avertex_shader::spawn();
          temp->load(name);
        });
    }
  }
}