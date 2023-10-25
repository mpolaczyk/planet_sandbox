#pragma once

#include "core/core.h"

#include "vec3.h"

class material;
class hittable;

namespace engine
{
  struct ENGINE_API hit_record
  {
    vec3 p;         // hit point
    vec3 normal;
    float t;        // distance to hit point
    float u;
    float v;
    const material* material_ptr = nullptr;
    bool front_face;
    hittable* object = nullptr;
    int face_id = 0;
  };
}