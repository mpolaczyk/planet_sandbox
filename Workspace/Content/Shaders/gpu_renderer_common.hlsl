
struct VS_Input
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD;
};

struct VS_Output
{
    float4 pos_ws    : TEXCOORD0;
    float3 normal_ws : TEXCOORD1;
    float2 uv        : TEXCOORD2;
    float4 pos       : SV_Position;
};