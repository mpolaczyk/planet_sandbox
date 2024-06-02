#include "deferred_common.hlsl"

Texture2D texture0 : register(t0);
sampler sampler0   : register(s0);

cbuffer fobject_data : register(b0)
{
  matrix model_world;
  matrix inverse_transpose_model_world;
  matrix model_world_view_projection;
  uint material_id;
};

struct PS_Output
{
  float4 position_ws : SV_Target0;
  float4 normal_ws	 : SV_Target1;
  float4 tex_color	 : SV_Target2;
  uint material_id   : SV_Target3;
};

PS_Output ps_main(VS_gbuffer_output input) : SV_Target
{
  PS_Output output;
  output.position_ws = input.position_ws;
  output.normal_ws   = float4(input.normal_ws, 1.0);
  output.tex_color   = texture0.Sample(sampler0, input.uv);
  output.material_id = material_id;
  return output;
}
