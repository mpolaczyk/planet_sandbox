#pragma once

#include "core/core.h"

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
}