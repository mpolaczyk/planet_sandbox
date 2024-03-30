#pragma once

#include <DirectXMath.h>

#include "core/core.h"
#include "math/vec3.h"

namespace engine
{
  ALIGN(64) struct ENGINE_API ftriangle_face
  {
    fvec3 vertices[3];
    fvec3 pad1;
    fvec3 normals[3];  // vertex normals
    fvec3 pad2;
    fvec3 UVs[3]; //xy
    fvec3 pad3;
  };

  struct ENGINE_API fvertex_data
  {
    fvertex_data(const DirectX::XMFLOAT3& in_pos, const DirectX::XMFLOAT3& in_norm, const DirectX::XMFLOAT2& in_uv)
      : pos(in_pos), uv(in_uv), norm(in_norm) {}

    DirectX::XMFLOAT3 pos = {0.f, 0.f, 0.f};
    DirectX::XMFLOAT2 uv = {0.f, 0.f};
    DirectX::XMFLOAT3 norm = {0.f, 0.f, 0.f};
  };
}