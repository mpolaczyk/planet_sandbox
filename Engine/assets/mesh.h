#pragma once

#include <directxcollision.h>

#include "core/core.h"
#include "object/object.h"
#include "asset/asset.h"
#include "renderer/render_state.h"

namespace engine
{
  class ENGINE_API astatic_mesh : public aasset_base
  {
  public:
    OBJECT_DECLARE(astatic_mesh, aasset_base)
    OBJECT_DECLARE_LOAD(astatic_mesh)
    OBJECT_DECLARE_SAVE(astatic_mesh)
    OBJECT_DECLARE_VISITOR

    virtual const char* get_extension() override
    {
      return ".mesh";
    }
    
    // JSON persistent
    std::string obj_file_name;

    // Runtime state
    DirectX::BoundingBox bounding_box;
    fstatic_mesh_render_state render_state;
  };
}
