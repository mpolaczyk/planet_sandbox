#pragma once

#include "core/core.h"

#include "math/vec3.h"
#include "math/aabb.h"

#include "object/object.h"

namespace engine
{
  class ENGINE_API hhittable_base : public oobject
  {
  public:
    OBJECT_DECLARE(hhittable_base, oobject)
    OBJECT_DECLARE_VISITOR

    virtual uint32_t get_hash() const override;

    virtual void load_resources()
    {
    };

    // Persistent members
    fvec3 origin = fvec3(0.0f, 0.0f, 0.0f);
    fvec3 scale = fvec3(1.0f, 1.0f, 1.0f);
    fvec3 rotation = fvec3(0.0f, 0.0f, 0.0f); // degrees

    // Runtime members
    faabb bounding_box;
  };
}
