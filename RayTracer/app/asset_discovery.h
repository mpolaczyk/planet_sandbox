#pragma once

#include <string>

class material;
class mesh;

class asset_discovery
{
public:
  static material* load_material(const std::string& material_name);
  static mesh* load_mesh(const std::string& mesh_name);

  static void save_material(const material* object);
};