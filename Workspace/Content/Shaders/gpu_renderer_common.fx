
struct VS_Input {
    float3 pos  : POSITION;
    float3 norm : NORMAL;
    float2 uv   : TEXCOORD;
};

struct VS_Output {
    float4 pos_ws    : TEXCOORD1;
    float3 normal_ws : TEXCOORD2;
    float2 uv        : TEXCOORD0;
    float4 pos       : SV_Position;
};