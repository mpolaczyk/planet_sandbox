#pragma once

#include "core/core.h"

#include "math/vec3.h"
#include "math/aabb.h"

#include "object/object.h"

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"

namespace engine
{
  class ENGINE_API hhittable_base : public oobject
  {
  public:
    OBJECT_DECLARE(hhittable_base, oobject)
    OBJECT_DECLARE_VISITOR

    virtual uint32_t get_hash() const;
    virtual hhittable_base* clone() const = 0;
    virtual void load_resources();

    // Persistent members
    fvec3 origin = fvec3(0, 0, 0);
    fvec3 scale = fvec3(1, 1, 1);
    fvec3 rotation = fvec3(0, 0, 0);  // degrees
    fsoft_asset_ptr<amaterial> material_asset_ptr;
    
    // Runtime members
    faabb bounding_box;
  };
}