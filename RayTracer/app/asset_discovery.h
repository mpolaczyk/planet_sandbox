#pragma once

#include <string>

class material;
class mesh;
class texture;

class asset_discovery
{
public:
  static material* load_material(const std::string& material_name);
  static mesh* load_mesh(const std::string& mesh_name);
  static texture* load_texture(const std::string& texture_name);

  static void save_material(const material* object);
};