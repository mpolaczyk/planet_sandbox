#pragma once

#include <DirectXMath.h>
#include "core/core.h"

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16
#define MAX_TEXTURES 1

static_assert(MAX_TEXTURES < 256);  // uint8_t limit
static_assert(MAX_MATERIALS < 256); // uint8_t limit

namespace engine
{
  using namespace DirectX;

  ALIGNED_STRUCT_BEGIN(fmaterial_properties)
  {
    XMFLOAT4 emissive{0.0f, 0.0f, 0.0f, 1.0f}; // 16
    //
    XMFLOAT4 ambient{0.0f, 0.0f, 0.0f, 1.0f}; // 16
    //
    XMFLOAT4 diffuse{0.0f, 0.0f, 0.0f, 1.0f}; // 16
    //
    XMFLOAT4 specular{0.0f, 0.0f, 0.0f, 1.0f}; // 16
    //
    float specular_power{1.0f}; // 4  
    int32_t use_texture{false}; // 4  
    float padding[2]; // 8
  };

  ALIGNED_STRUCT_END(fmaterial_properties)

  enum ENGINE_API flight_type
  {
    point = 0,
    directional,
    spotlight
  };

  ALIGNED_STRUCT_BEGIN(flight_properties)
  {
    XMFLOAT4 position{0.0f, 0.0f, 0.0f, 1.0f}; // 16
    //
    XMFLOAT4 direction{0.0f, 0.0f, 0.0f, 0.0f}; // 16
    //
    XMFLOAT4 color{0.0f, 0.0f, 0.0f, 1.0f}; // 16
    //
    float spot_angle{XM_PIDIV2}; // 4 
    float constant_attenuation{1.0f}; // 4 
    float linear_attenuation{0.0f}; // 4 
    float quadratic_attenuation{0.0f}; // 4
    //
    int32_t light_type{flight_type::point}; // 4 
    int32_t enabled{0}; // 4 
    int32_t padding[2]; // 8
  };

  ALIGNED_STRUCT_END(flight_properties)
}