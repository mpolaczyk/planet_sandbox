
struct fvs_input
{
  float3 position  : POSITION;
  float3 normal    : NORMAL;
  float3 tangent   : TANGENT;
  float3 bitangent : BITANGENT;
  float2 uv        : TEXCOORD;
};

struct fvs_output
{
  float4 position_cs  : SV_Position;
  float4 position_ws  : TEXCOORD0;
  float3 normal_ws    : TEXCOORD1;
  float2 uv           : TEXCOORD2;
};

struct fps_output
{
  float4 position_ws : SV_Target0;
  float4 normal_ws	 : SV_Target1;
  float2 uv          : SV_Target2;
  uint material_id   : SV_Target3;
};

struct fobject_data
{
  matrix model_world;
  matrix inverse_transpose_model_world;
  matrix model_world_view_projection;
  uint material_id;
};

ConstantBuffer<fobject_data> object_data : register(b0);

fvs_output vs_main(fvs_input input)
{
  fvs_output output;
  output.position_cs = mul(object_data.model_world_view_projection, float4(input.position, 1.0f));
  output.position_ws = mul(object_data.model_world, float4(input.position, 1.0f));
  output.normal_ws   = normalize(mul((float3x3)object_data.inverse_transpose_model_world, input.normal));
  output.uv          = input.uv;
  return output;
}

fps_output ps_main(fvs_output input) : SV_Target
{
  fps_output output;
  output.position_ws = input.position_ws;
  output.normal_ws   = float4(input.normal_ws, 1.0);
  output.uv          = input.uv;
  output.material_id = object_data.material_id;
  return output;
}