#include "gpu_renderer_common.hlsl"

Texture2D    texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 ps_main(VS_Output input) : SV_Target
{
    return texture0.Sample(sampler0, input.uv);   
}
