#pragma once

#include <string>
#include <vector>

#include "core/core.h"

#include "asset/asset.h"
#include "math/vec3.h"
#include "math/triangle_face.h"

namespace engine
{
  class ENGINE_API mesh : public asset
  {
  public:
    static asset_type get_static_asset_type();
    static mesh* load(const std::string& mesh_name);
    static void save(mesh* object);
    static mesh* spawn();

    // JSON persistent
    std::string obj_file_name;
    int shape_index;

    // OBJ resource
    std::vector<triangle_face> faces;
  };
}

