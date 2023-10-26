#pragma once

#include <string>

#include "engine.h"

class asset_discovery
{
public:
  static engine::material* load_material(const std::string& material_name);
  static engine::mesh* load_mesh(const std::string& mesh_name);
  static engine::texture* load_texture(const std::string& texture_name);

  static void save_material(const engine::material* object);
};