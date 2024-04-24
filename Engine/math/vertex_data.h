#pragma once

#include <DirectXMath.h>

#include "core/core.h"
#include "math/vec3.h"

namespace engine
{
  using namespace DirectX;
  
  ALIGN(64) struct ENGINE_API ftriangle_face  // Used by cpu renderers
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
    fvertex_data() = default;
    fvertex_data(const fvertex_data&) = default;
    fvertex_data& operator=(const fvertex_data&) = default;
    fvertex_data(fvertex_data&&) = default;
    fvertex_data& operator=(fvertex_data&&) = default;
    
    fvertex_data(const XMFLOAT3& in_position, const XMFLOAT3& in_normal, const XMFLOAT3& in_tangent, const XMFLOAT3& in_bitangent, const XMFLOAT2& in_uv)
      : position(in_position), normal(in_normal), tangent(in_tangent), bitangent(in_bitangent), uv(in_uv) {}
    
    XMFLOAT3 position = {0.f, 0.f, 0.f};
    XMFLOAT3 normal = {0.f, 0.f, 0.f};
    XMFLOAT3 tangent = {0.f, 0.f, 0.f};
    XMFLOAT3 bitangent = {0.f, 0.f, 0.f};
    XMFLOAT2 uv = {0.f, 0.f};
  };

  typedef uint32_t fface_data_type;   // Remember to match what is in IASetIndexBuffer
  struct ENGINE_API fface_data
  {
    fface_data() = default;
    fface_data(const fface_data&) = default;
    fface_data& operator=(const fface_data&) = default;
    fface_data(fface_data&&) = default;
    fface_data& operator=(fface_data&&) = default;
    
    fface_data(fface_data_type in_v1, fface_data_type in_v2, fface_data_type in_v3)
      : v1(in_v1), v2(in_v2), v3(in_v3) {}
    
    fface_data_type v1;
    fface_data_type v2;
    fface_data_type v3;
  };
}