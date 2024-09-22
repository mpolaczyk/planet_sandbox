#pragma once

#include <DirectXMath.h>
#include "core/core.h"

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16
#define MAX_TEXTURES 32
#define MAX_MAIN_DESCRIPTORS 128

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
    int32_t texture_id{-1}; // 4
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

  ALIGNED_STRUCT_BEGIN(fframe_data)
  {
    XMFLOAT4 camera_position; // 16
    XMFLOAT4 ambient_light; // 16
    int32_t show_emissive; // 4    // TODO pack bits
    int32_t show_ambient; // 4
    int32_t show_specular; // 4
    int32_t show_diffuse; // 4
    int32_t show_normals; // 4
    int32_t show_object_id; // 4
    int32_t padding[2]; // 8
  };

  ALIGNED_STRUCT_END(fframe_data)

  ALIGNED_STRUCT_BEGIN(fobject_data)
  {
    XMFLOAT4X4 model_world; // 64 Used to transform the vertex position from object space to world space
    XMFLOAT4X4 inverse_transpose_model_world; // 64 Used to transform the vertex normal from object space to world space
    XMFLOAT4X4 model_world_view_projection; // 64 Used to transform the vertex position from object space to projected clip space
    XMFLOAT4 object_id; // 16
    uint8_t material_id; // 1
    uint8_t is_selected; // 1
    uint8_t padding[14]; // 1
  };
  static_assert(sizeof(fobject_data)/4 < 64); // "Root Constant size is greater than 64 DWORDs. Additional indirection may be added by the driver."
  ALIGNED_STRUCT_END(fobject_data)
}