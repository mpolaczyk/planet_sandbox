#pragma once

#include <DirectXMath.h>
#include "core/core.h"

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16
#define MAX_TEXTURES 32
#define MAX_MAIN_DESCRIPTORS 128    // TODO those should not be here, fpass_base?
#define MAX_RTV_DESCRIPTORS 8
#define MAX_DSV_DESCRIPTORS 4

namespace engine
{
  using namespace DirectX;

  // Nsight type: vec4f emissive; vec4f ambient; vec4f diffuse; vec4f specular; float specular_power; int32 texture_id; float pad; float pad;
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

  // Nsight type: vec4f position; vec4f direction; vec4f color; float spot_angle; float constant_attenuation; float linear_attenuation; float quadratic_attenuation; int32 light_type; int32 enabled; int64 pad;
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

  // Warning! object data is shared by all rendring passes because it is used in the fscene_acceleration
  ALIGNED_STRUCT_BEGIN(fobject_data)
  {
    XMFLOAT4X4 model_world; // 64 Used to transform the vertex position from object space to world space
    XMFLOAT4X4 inverse_transpose_model_world; // 64 Used to transform the vertex normal from object space to world space
    XMFLOAT4X4 model_world_view_projection; // 64 Used to transform the vertex position from object space to projected clip space
    uint32_t material_id; // 4
    uint32_t is_selected; // 4
  };
  ALIGNED_STRUCT_END(fobject_data)
  static_assert(sizeof(fobject_data)/4 < 64); // "Root Constant size is greater than 64 DWORDs. Additional indirection may be added by the driver."

  // Warning! frame data is shared by multiple passes in a multiple ways: root constants or root constant buffer parameter
  ALIGNED_STRUCT_BEGIN(fframe_data)
  {
    XMFLOAT4 camera_position; // 16
    XMFLOAT4 ambient_light;   // 16
    uint32_t height;          // 4
    uint32_t width;           // 4
    uint32_t pad[2];          // 8
  };
  ALIGNED_STRUCT_END(fframe_data)
  static_assert(sizeof(fframe_data)/4 < 64); // "Root Constant size is greater than 64 DWORDs. Additional indirection may be added by the driver."
}