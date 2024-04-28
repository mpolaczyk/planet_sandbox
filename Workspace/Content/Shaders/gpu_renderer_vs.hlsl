#include "gpu_renderer_common.hlsl"

cbuffer fvs_object_data : register(b0)
{
    matrix model_world;
    matrix inverse_transpose_model_world;
    matrix model_world_view_projection;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position_ws    = mul(model_world, float4(input.position, 1.0f));
    output.position       = mul(model_world_view_projection, float4(input.position, 1.0f));
    output.normal_ws      = mul((float3x3)inverse_transpose_model_world, input.normal);
    output.uv             = input.uv;
    return output;
}
