
struct VS_Input {
    float3 pos  : POS;
    float2 uv   : TEX;
    float3 norm : NORM;
};

struct VS_Output {
    float4 pos_ws    : TEXCOORD1;
    float3 normal_ws : TEXCOORD2;
    float2 uv        : TEXCOORD0;
    float4 pos       : SV_Position;
    //float4 pos : SV_POSITION;
    //float2 uv : TEXCOORD;
};