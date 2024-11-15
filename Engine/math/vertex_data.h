#pragma once

#include <DirectXMath.h>

#include "core/core.h"
#include "math/vec3.h"

namespace engine
{
  using namespace DirectX;

  ALIGN(64) struct ENGINE_API ftriangle_face
  {
    fvec3 vertices[3];
    fvec3 pad1;
    fvec3 normals[3]; // vertex normals
    fvec3 pad2;
    fvec3 UVs[3]; //xy
    fvec3 pad3;
  };

  struct ENGINE_API fvertex_data
  {
    CTOR_DEFAULT(fvertex_data)
    CTOR_MOVE_COPY_DEFAULT(fvertex_data)
    DTOR_DEFAULT(fvertex_data)

    fvertex_data(const XMFLOAT3& in_position, const XMFLOAT3& in_normal, const XMFLOAT3& in_tangent, const XMFLOAT3& in_bitangent, const XMFLOAT2& in_uv)
      : position(in_position), normal(in_normal), tangent(in_tangent), bitangent(in_bitangent), uv(in_uv)
    {
    }

    // Don't change the layout, this needs to match the input layout for vertex buffers
    XMFLOAT3 position = {0.f, 0.f, 0.f};
    XMFLOAT3 normal = {0.f, 0.f, 0.f};
    XMFLOAT3 tangent = {0.f, 0.f, 0.f};
    XMFLOAT3 bitangent = {0.f, 0.f, 0.f};
    XMFLOAT2 uv = {0.f, 0.f};
  };

  typedef uint32_t fface_data_type; // Remember to match what is in IASetIndexBuffer
  struct ENGINE_API fface_data
  {
    fface_data_type v1 = 0;
    fface_data_type v2 = 0;
    fface_data_type v3 = 0;
  };
}
