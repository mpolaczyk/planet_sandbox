#define MAX_MATERIALS 32
#define MAX_LIGHTS 16

struct VS_Input
{
  float3 position  : POSITION;
  float3 normal    : NORMAL;
  float3 tangent   : TANGENT;
  float3 bitangent : BITANGENT;
  float2 uv        : TEXCOORD;
};

struct VS_output
{
  float4 position_cs  : SV_Position;
  float4 position_ws  : TEXCOORD0;
  float3 normal_ws    : TEXCOORD1;
  float2 uv           : TEXCOORD2;
};

struct PS_Output
{
  float4 position_ws : SV_Target0;
  float4 normal_ws	 : SV_Target1;
  float4 tex_color	 : SV_Target2;
  uint material_id   : SV_Target3;
};

Texture2D texture0 : register(t0);
sampler sampler0   : register(s0);

cbuffer fobject_data : register(b0)
{
  matrix model_world;
  matrix inverse_transpose_model_world;
  matrix model_world_view_projection;
  uint material_id;
};

VS_output vs_main(VS_Input input)
{
  VS_output output;
  output.position_cs = mul(model_world_view_projection, float4(input.position, 1.0f));
  output.position_ws = mul(model_world, float4(input.position, 1.0f));
  output.normal_ws   = normalize(mul((float3x3)inverse_transpose_model_world, input.normal));
  output.uv          = input.uv;
  return output;
}

PS_Output ps_main(VS_output input) : SV_Target
{
  PS_Output output;
  output.position_ws = input.position_ws;
  output.normal_ws   = float4(input.normal_ws, 1.0);
  output.tex_color   = texture0.Sample(sampler0, input.uv);
  output.material_id = material_id;
  return output;
}