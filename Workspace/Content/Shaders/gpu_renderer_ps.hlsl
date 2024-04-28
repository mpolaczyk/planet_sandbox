#include "gpu_renderer_common.hlsl"

Texture2D    texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer fps_frame_data : register(b0)
{
    matrix view_projection;     // 64
    //
    float4 ambient_light;       // 16
    //
    flight_properties light;    // 80
};

float4 ps_main(VS_Output input) : SV_Target
{
    const float4 light_dir = light.position - input.position_ws;
    
    return ambient_light + light.color * saturate(dot(normalize(light_dir), input.normal_ws));
    
    //return float4(input.normal_ws * 0.5 + 0.5, 1);
    //return texture0.Sample(sampler0, input.uv);
}
