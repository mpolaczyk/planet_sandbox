#include "material_properties.hlsl"

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16

struct VS_Output
{
  float4 position_cs  : SV_Position;
  float4 position_ws  : TEXCOORD0;
  float3 normal_ws    : TEXCOORD1;
  float2 uv           : TEXCOORD2;
};

struct flight_properties
{
  float4 position;                // 16
  //
  float4 direction;               // 16
  //
  float4 color;                   // 16
  //
  float spot_angle;               // 4
  float constant_attenuation;     // 4
  float linear_attenuation;       // 4
  float quadratic_attenuation;    // 4
  //
  int light_type;                 // 4
  int enabled;                    // 4
  int2 padding;                   // 8
};