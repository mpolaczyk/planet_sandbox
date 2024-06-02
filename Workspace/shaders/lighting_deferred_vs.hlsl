#include "deferred_common.hlsl"
#include "material_properties.hlsl"
#include "light_properties.hlsl"

struct VS_Input
{
  float3 position  : POSITION;
  float3 normal    : NORMAL;
  float3 tangent   : TANGENT;
  float3 bitangent : BITANGENT;
  float2 uv        : TEXCOORD;
};

cbuffer fframe_data : register(b0)
{
  float4 camera_position;     // 16
  float4 ambient_light;       // 16
  matrix model_world_view_projection; // 64
  int show_position_ws;			// 4        // TODO bit flags
  int show_normal_ws;			  // 4
  int show_tex_color;			  // 4
  int padding;              // 4
  flight_properties lights[MAX_LIGHTS];    // 80xN
  fmaterial_properties materials[MAX_MATERIALS];  // 80xN
};

VS_lighting_output vs_main(VS_Input input)
{
  VS_lighting_output output;
  output.position_cs = float4(input.position, 1.0f);
  output.uv          = input.uv;
  return output;
}
