#include "gpu_renderer_common.fx"

cbuffer per_object_data : register(b0)
{
    matrix world;
    matrix inverse_transpose_world;
    matrix world_view_projection;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    output.pos       = mul( world_view_projection, float4( input.pos, 1.0f ) );
    output.pos_ws    = mul( world, float4( input.pos, 1.0f ) );
    output.normal_ws = mul( (float3x3)inverse_transpose_world, input.norm );
    output.uv        = input.uv;

    return output;
}
