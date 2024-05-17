#pragma once

#include "core/core.h"

#include "math/vec3.h"

namespace engine
{
  class amaterial;
  class hhittable_base;

  struct ENGINE_API fhit_record
  {
    fvec3 p; // hit point
    fvec3 normal;
    float t; // distance to hit point
    float u;
    float v;
    const amaterial* material_ptr = nullptr;
    bool front_face;
    hhittable_base* object;
    int face_id = 0;
  };
}
