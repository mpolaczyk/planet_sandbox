#include "gpu_renderer_common.hlsl"

cbuffer per_frame_data : register(b0)
{
    matrix view_projection;
};

cbuffer per_object_data : register(b1)
{
    matrix world;
    matrix inverse_transpose_world;
    matrix world_view_projection;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(float4(input.pos, 1.0f), world_view_projection);
    output.uv = input.uv;
    return output;
    
    //output.pos       = mul(world_view_projection, float4(input.pos, 1.0f));
    //output.pos_ws    = mul(world, float4(input.pos, 1.0f));
    //output.normal_ws = mul((float3x3)inverse_transpose_world, input.norm);
    //output.uv        = input.uv;
    //
    //return output;
}
