#pragma once

#include "app/asset.h"
#include "math/vec3.h"

class mesh : public asset
{
public:
  static asset_type get_static_asset_type();
  static mesh* load(const std::string& mesh_name);
  static void save(mesh* object) {};
  static mesh* spawn();

  // JSON persistent
  std::string obj_file_name;
  int shape_index;

  // OBJ resource
  std::vector<triangle_face> faces;
};

