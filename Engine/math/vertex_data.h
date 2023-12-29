#pragma once

#include "core/core.h"
#include "math/vec3.h"

namespace engine
{
  ALIGN(64) struct ENGINE_API triangle_face  // todo move somewhere else
  {
    vec3 vertices[3];
    vec3 pad1;
    vec3 normals[3];  // vertex normals
    vec3 pad2;
    vec3 UVs[3]; //xy
    vec3 pad3;

    // TODO: compute face normal on load?
  };

  struct ENGINE_API vertex_data
  {
    float pos[3];
    float uv[2];
    float norm[3];
  };
}