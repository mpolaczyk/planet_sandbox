#pragma once

#include <DirectXMath.h>

#include "core/core.h"
#include "math/vec3.h"

namespace engine
{
  ALIGN(64) struct ENGINE_API triangle_face
  {
    vec3 vertices[3];
    vec3 pad1;
    vec3 normals[3];  // vertex normals
    vec3 pad2;
    vec3 UVs[3]; //xy
    vec3 pad3;
  };

  struct ENGINE_API vertex_data
  {
    DirectX::XMFLOAT3 pos = {0.f, 0.f, 0.f};
    DirectX::XMFLOAT2 uv = {0.f, 0.f};
    DirectX::XMFLOAT3 norm = {0.f, 0.f, 0.f};
  };
}