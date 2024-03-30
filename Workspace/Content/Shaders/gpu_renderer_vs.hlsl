#include "gpu_renderer_common.hlsl"

cbuffer fframe_data : register(b0)
{
    matrix view_projection;
};

cbuffer fobject_data : register(b1)
{
    matrix model_world;
    matrix transpose_inverse_model_world;
    matrix model_world_view_projection;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos_ws    = mul(model_world, float4(input.pos, 1.0f));
    output.normal_ws = mul((float3x3)transpose_inverse_model_world, input.normal);
    output.pos       = mul(float4(input.pos, 1.0f), model_world_view_projection);
    output.uv        = input.uv;
    
    return output;
}
