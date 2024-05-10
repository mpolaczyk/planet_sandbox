#include "gpu_renderer_common.hlsl"

cbuffer fobject_data : register(b0)
{
    matrix model_world;
    matrix inverse_transpose_model_world;
    matrix model_world_view_projection;
    int material_id;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position_ws    = mul(model_world, float4(input.position, 1.0f));
    output.position       = mul(model_world_view_projection, float4(input.position, 1.0f));
    output.normal_ws      = normalize(mul((float3x3)inverse_transpose_model_world, input.normal));
    output.uv             = input.uv;
    return output;
}
