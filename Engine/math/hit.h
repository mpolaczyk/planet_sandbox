#pragma once

#include "core/core.h"

#include "math/vec3.h"

namespace engine
{
  class material;

  struct ENGINE_API hit_record
  {
    vec3 p;         // hit point
    vec3 normal;
    float t;        // distance to hit point
    float u;
    float v;
    const material* material_ptr = nullptr;
    bool front_face;
    void* object = nullptr; //FIX
    int face_id = 0;
  };
}