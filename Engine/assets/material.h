#pragma once

#include <DirectXMath.h>

#include "core/core.h"

#include "object/object.h"
#include "math/vec3.h"
#include "asset/asset.h"

namespace engine
{
  using namespace DirectX;
  
  struct alignas(16) ENGINE_API fmaterial_properties
  {
    XMFLOAT4 emissive;        // 16
    //
    XMFLOAT4 ambient;         // 16
    //
    XMFLOAT4 diffuse;         // 16
    //
    XMFLOAT4 specular;        // 16
    //
    float specular_power;     // 4  
    int use_texture;          // 4  
    float padding[2];         // 8
  };
  static_assert(sizeof(fmaterial_properties) % 16 == 0);
  
  enum ENGINE_API flight_type
  {
    directional = 0,
    point,
    spotlight
  };
  
  struct alignas(16) ENGINE_API flight_properties
  {
    XMFLOAT4 position           { 0.0f, 0.0f, 0.0f, 1.0f };   // 16
    //
    XMFLOAT4 direction          { 0.0f, 0.0f, 1.0f, 0.0f };   // 16
    //
    XMFLOAT4 color              { 1.0f, 1.0f, 1.0f, 1.0f };   // 16
    //
    float spot_angle            { XM_PIDIV2 };                // 4 
    float constant_attenuation  { 1.0f };                     // 4 
    float linear_attenuation    { 0.0f };                     // 4 
    float quadratic_attenuation { 0.0f };                     // 4
    //
    int light_type              { flight_type::point };       // 4 
    int enabled                 { 0 };                        // 4 
    int padding[2];                                           // 8
  };
  static_assert(sizeof(flight_properties) % 16 == 0);
  
  class ENGINE_API amaterial : public aasset_base
  {
  public:
    OBJECT_DECLARE(amaterial, aasset_base)
    OBJECT_DECLARE_LOAD(amaterial)
    OBJECT_DECLARE_SAVE(amaterial)
    OBJECT_DECLARE_VISITOR
    
    // JSON persistent - CPU Raytracer
    bool is_light = false;
    fvec3 color;
    fvec3 emitted_color;
    fvec3 gloss_color;
    fvec3 pad;
    float smoothness = 0.0f;
    float gloss_probability = 0.0f;
    float refraction_probability = 0.0f;
    float refraction_index = 1.0f;
    // JSON persistent - GPU renderer
    fmaterial_properties material;
    flight_properties light;
  };
}
