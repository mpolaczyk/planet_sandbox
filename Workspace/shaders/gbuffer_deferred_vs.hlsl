#include "deferred_common.hlsl"

struct VS_Input
{
  float3 position  : POSITION;
  float3 normal    : NORMAL;
  float3 tangent   : TANGENT;
  float3 bitangent : BITANGENT;
  float2 uv        : TEXCOORD;
};

cbuffer fobject_data : register(b0)
{
  matrix model_world;
  matrix inverse_transpose_model_world;
  matrix model_world_view_projection;
  uint material_id;
};

VS_Output vs_main(VS_Input input)
{
  VS_Output output;
  output.position_cs = mul(model_world_view_projection, float4(input.position, 1.0f));
  output.position_ws = mul(model_world, float4(input.position, 1.0f));
  output.normal_ws   = normalize(mul((float3x3)inverse_transpose_model_world, input.normal));
  output.uv          = input.uv;
  return output;
}
