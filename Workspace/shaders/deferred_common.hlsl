
struct VS_Output
{
  float4 position_cs  : SV_Position;
  float4 position_ws  : TEXCOORD0;
  float3 normal_ws    : TEXCOORD1;
  float2 uv           : TEXCOORD2;
};