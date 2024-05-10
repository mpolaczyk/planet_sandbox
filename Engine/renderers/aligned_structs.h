#pragma once

#include <DirectXMath.h>
#include "core/core.h"

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16

#define ALIGNED_STRUCT_BEGIN(NAME) struct alignas(16) ENGINE_API NAME
#define ALIGNED_STRUCT_END(NAME) static_assert(sizeof(NAME) % 16 == 0);

namespace engine
{
    using namespace DirectX;
    
    ALIGNED_STRUCT_BEGIN(fmaterial_properties)
    {
        XMFLOAT4 emissive         { 0.0f, 0.0f, 0.0f, 1.0f }; // 16
        //
        XMFLOAT4 ambient          { 0.0f, 0.0f, 0.0f, 1.0f }; // 16
        //
        XMFLOAT4 diffuse          { 0.0f, 0.0f, 0.0f, 1.0f }; // 16
        //
        XMFLOAT4 specular         { 0.0f, 0.0f, 0.0f, 1.0f }; // 16
        //
        float specular_power      { 1.0f };                   // 4  
        int use_texture           { false };                  // 4  
        float padding[2];                                     // 8
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
        XMFLOAT4 position           { 0.0f, 0.0f, 0.0f, 1.0f };   // 16
        //
        XMFLOAT4 direction          { 0.0f, 0.0f, 0.0f, 0.0f };   // 16
        //
        XMFLOAT4 color              { 0.0f, 0.0f, 0.0f, 1.0f };   // 16
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
    
    ALIGNED_STRUCT_BEGIN(fframe_data)
    {
        XMFLOAT4 camera_position;                       // 16
        XMFLOAT4 ambient_light;                         // 16
        int show_emissive;                              // 4
        int show_ambient;                               // 4
        int show_specular;                              // 4
        int show_diffuse;                               // 4
        int show_normals;                               // 4
        int padding[3];                                 // 12
        flight_properties lights[MAX_LIGHTS];           // 80xN
        fmaterial_properties materials[MAX_MATERIALS];  // 80xN
    };
    ALIGNED_STRUCT_END(fframe_data)

    ALIGNED_STRUCT_BEGIN(fobject_data)
    {
        XMFLOAT4X4 model_world;                     // 64 Used to transform the vertex position from object space to world space
        XMFLOAT4X4 inverse_transpose_model_world;   // 64 Used to transform the vertex normal from object space to world space
        XMFLOAT4X4 model_world_view_projection;     // 64 Used to transform the vertex position from object space to projected clip space
        int material_id;                            // 4
        int is_selected;                            // 4
        int padding[2];                             // 8
    };
    ALIGNED_STRUCT_END(fobject_data)
}