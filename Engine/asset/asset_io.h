#pragma once

#include <string>

#include "core/core.h"

namespace engine
{
  class material_asset;
  class static_mesh_asset;
  class texture_asset;

  class ENGINE_API asset_io
  {
  public:
    static material_asset* load_material_asset(const std::string& material_name);
    static static_mesh_asset* load_static_mesh_asset(const std::string& mesh_name);
    static texture_asset* load_texture_asset(const std::string& texture_name);

    static void save_material_asset(const material_asset* object);
  };
}