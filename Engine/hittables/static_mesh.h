#pragma once

#include <string>

#include "hittables.h"
#include "math/vec3.h"
#include "asset/soft_asset_ptr.h"
#include "assets/mesh.h"

namespace engine
{
  class ENGINE_API hstatic_mesh : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hstatic_mesh, hhittable_base)
    OBJECT_DECLARE_VISITOR

    virtual uint32_t get_hash() const override;
    virtual void load_resources() override;

    // Persistent state
    fsoft_asset_ptr<astatic_mesh> mesh_asset_ptr;
  };
}