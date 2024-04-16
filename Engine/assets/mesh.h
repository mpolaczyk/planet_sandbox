#pragma once

#include <d3d11.h>
#include <directxcollision.h>
#include <wrl/client.h>
#include <vector>

#include "core/core.h"

#include "object/object.h"
#include "math/vertex_data.h"
#include "asset/asset.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct ENGINE_API fstatic_mesh_render_state
  {
    unsigned int num_indices;
    unsigned int stride;
    unsigned int offset;
    ComPtr<ID3D11Buffer> vertex_buffer;
    ComPtr<ID3D11Buffer> index_buffer;
  };
  
  class ENGINE_API astatic_mesh : public aasset_base
  {
  public:
    OBJECT_DECLARE(astatic_mesh, aasset_base)
    OBJECT_DECLARE_LOAD(astatic_mesh)
    OBJECT_DECLARE_SAVE(astatic_mesh)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string obj_file_name;

    // Runtime state
    std::vector<ftriangle_face> faces; // for CPU renderers only
    DirectX::BoundingBox bounding_box;
    fstatic_mesh_render_state render_state;
  };
}

