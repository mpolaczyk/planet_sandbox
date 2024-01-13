
float4 ps_main(VS_Output input) : SV_Target
{
    return mytexture.Sample(mysampler, input.uv);   
}
