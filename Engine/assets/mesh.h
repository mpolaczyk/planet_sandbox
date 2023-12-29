#pragma once

#include <directxcollision.h>
#include <vector>

#include "core/core.h"

#include "object/object.h"
#include "math/vertex_data.h"
#include "asset/asset.h"

struct ID3D11Buffer;

namespace engine
{
  struct static_mesh_render_state
  {
    unsigned int num_indices;
    unsigned int stride;
    unsigned int offset;
    ID3D11Buffer* vertex_buffer;
    ID3D11Buffer* index_buffer;
  };
  
  class ENGINE_API static_mesh_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(static_mesh_asset, asset_base)
    OBJECT_DECLARE_LOAD(static_mesh_asset)
    OBJECT_DECLARE_SAVE(static_mesh_asset)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string obj_file_name;

    // Runtime state
    std::vector<triangle_face> faces; // for CPU renderers only
    DirectX::BoundingBox bounding_box;
    static_mesh_render_state render_state;
  };
}

