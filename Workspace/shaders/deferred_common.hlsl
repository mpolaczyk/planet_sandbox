
struct VS_gbuffer_output
{
  float4 position_cs  : SV_Position;
  float4 position_ws  : TEXCOORD0;
  float3 normal_ws    : TEXCOORD1;
  float2 uv           : TEXCOORD2;
};

struct VS_lighting_output
{
  float4 position_cs : SV_POSITION;
  float2 uv	         : TEXCOORD;
};

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16