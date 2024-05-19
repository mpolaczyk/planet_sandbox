
struct VS_Input
{
    float3 position  : POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv        : TEXCOORD;
};

struct VS_Output
{
    float4 position_ws  : TEXCOORD0;
    float3 normal_ws    : TEXCOORD1;
    float2 uv           : TEXCOORD2;
    float4 position     : SV_Position;
};