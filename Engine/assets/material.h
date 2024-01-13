#pragma once

#include <DirectXMath.h>

#include "core/core.h"

#include "object/object.h"
#include "math/vec3.h"
#include "asset/asset.h"

namespace engine
{
  struct material_properties
  {
    DirectX::XMFLOAT4 emissive;   // 16 16  - 1
    DirectX::XMFLOAT4 ambient;    // 16 32  - 2
    DirectX::XMFLOAT4 diffuse;    // 16 48
    DirectX::XMFLOAT4 specular;   // 16 64  - 3
    float specular_power;         // 4  68
    int use_texture;              // 4  72
    float padding[2];             // 8  80  - 4
  };

  enum light_type
  {
    directional = 0,
    point,
    spotlight
  };
  
  struct light_properties
  {
    DirectX::XMFLOAT4 position  { 0.0f, 0.0f, 0.0f, 1.0f };   // 16 16  - 1
    DirectX::XMFLOAT4 direction { 0.0f, 0.0f, 1.0f, 0.0f };   // 16 32  - 2
    DirectX::XMFLOAT4 color     { 1.0f, 1.0f, 1.0f, 1.0f };   // 16 48  - 3
    float spot_angle            { DirectX::XM_PIDIV2 };       // 4 52
    float constant_attenuation  { 1.0f };                     // 4 56
    float linear_attenuation    { 0.0f };                     // 4 60
    float quadratic_attenuation { 0.0f };                     // 4 64  - 4
    int light_type              { light_type::point };        // 4 68
    int enabled                 { 0 };                        // 4 72
    int padding[2];                                           // 8 80  - 5
  };
  
  class ENGINE_API material_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(material_asset, asset_base)
    OBJECT_DECLARE_LOAD(material_asset)
    OBJECT_DECLARE_SAVE(material_asset)
    OBJECT_DECLARE_VISITOR

    // FIX Move to this class:
    // - material from hittable
    // - shaders from renderer
    // Use those in the gpu renderer
    
    // JSON persistent - CPU Raytracer
    bool is_light = false;
    vec3 color;
    vec3 emitted_color;
    vec3 gloss_color;
    vec3 pad;
    float smoothness = 0.0f;
    float gloss_probability = 0.0f;
    float refraction_probability = 0.0f;
    float refraction_index = 1.0f;
    // JSON persistent - GPU renderer
    material_properties material_props;
    light_properties light_props;
  };
}
