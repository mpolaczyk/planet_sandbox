#include "gpu_renderer_common.hlsl"

Texture2D    texture0 : register(t0);
SamplerState sampler0 : register(s0);


float4 ps_main(VS_Output input) : SV_Target
{
    const float4 diffuse = { 0.9, 0.9, 0.7, 1.0};
    const float4 ambient = {0.2, 0.2, 0.2, 1.0};
    
    float4 light_loc = {0,10,0,0};
    float4 light_dir = light_loc - input.position_ws;
    
    return ambient + diffuse * saturate(dot(normalize(light_dir), input.normal_ws));
    
    //return float4(input.normal_ws * 0.5 + 0.5, 1);
    //return texture0.Sample(sampler0, input.uv);
}
