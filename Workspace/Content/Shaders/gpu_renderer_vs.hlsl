
cbuffer per_object_constants : register(b0)
{
    matrix world;
    matrix inverse_transpose_world;
    matrix world_view_projection;
};

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

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    output.pos       = mul( world_view_projection, float4( input.pos, 1.0f ) );
    output.pos_ws    = mul( world, float4( input_pos, 1.0f ) );
    output.normal_ws = mul( (float3x3)inverse_transpose_world, input.norm );
    output.uv        = input.uv;

    return output;
}
