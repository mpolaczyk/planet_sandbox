#include "gpu_renderer_common.fx"

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

float4 ps_main(VS_Output input) : SV_Target
{
    return mytexture.Sample(mysampler, input.uv);   
}
