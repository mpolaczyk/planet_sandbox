#pragma once

#include "core/core.h"

#include "math/vec3.h"

namespace engine
{
  class material_asset;
  class hittable;

  struct ENGINE_API hit_record
  {
    vec3 p;         // hit point
    vec3 normal;
    float t;        // distance to hit point
    float u;
    float v;
    const material_asset* material_ptr = nullptr;
    bool front_face;
    hittable* object;
    int face_id = 0;
  };
}