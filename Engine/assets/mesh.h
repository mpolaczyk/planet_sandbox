#pragma once

#include <directxcollision.h>
#include <vector>

#include "core/core.h"

#include "object/object.h"
#include "math/vertex_data.h"
#include "asset/asset.h"

namespace engine
{
  class ENGINE_API static_mesh_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(static_mesh_asset, asset_base)
    OBJECT_DECLARE_LOAD(static_mesh_asset)
    OBJECT_DECLARE_SAVE(static_mesh_asset)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string obj_file_name;

    // OBJ resource
    std::vector<triangle_face> faces; // for CPU renderers
    std::vector<vertex_data> vertex_buffer;
    std::vector<uint16_t> index_buffer;
    DirectX::BoundingBox bounding_box;
  };
}

