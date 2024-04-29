#pragma once

#include <DirectXMath.h>
#include "core/core.h"

#define ALIGNED_STRUCT_BEGIN(NAME) struct alignas(16) ENGINE_API NAME
#define ALIGNED_STRUCT_END(NAME) static_assert(sizeof(NAME) % 16 == 0);

namespace engine
{
    using namespace DirectX;
    
    ALIGNED_STRUCT_BEGIN(fmaterial_properties)
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
    ALIGNED_STRUCT_END(fmaterial_properties)
  
    enum ENGINE_API flight_type
    {
        directional = 0,
        point,
        spotlight
    };
  
    ALIGNED_STRUCT_BEGIN(flight_properties)
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
    ALIGNED_STRUCT_END(flight_properties)
    
    ALIGNED_STRUCT_BEGIN(fframe_data)  // WARNING - const buffer type
    {
        XMFLOAT4X4 view_projection;
        XMFLOAT4 ambient_light;
        flight_properties light;
    };
    ALIGNED_STRUCT_END(fframe_data)

    ALIGNED_STRUCT_BEGIN(fobject_data) // WARNING - const buffer type
    {
        XMFLOAT4X4 model_world;                    // Used to transform the vertex position from object space to world space
        XMFLOAT4X4 inverse_transpose_model_world;  // Used to transform the vertex normal from object space to world space
        XMFLOAT4X4 model_world_view_projection;    // Used to transform the vertex position from object space to projected clip space
    };
    ALIGNED_STRUCT_END(fobject_data)
}