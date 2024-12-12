
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
};

struct fobject_data 
{
  matrix model_world;
  matrix inverse_transpose_model_world;
  matrix model_world_view_projection;
  uint material_id;
};

struct fframe_data
{
  float4 camera_position;     // 16
};

ConstantBuffer<fobject_data> object_data : register(b0);
ConstantBuffer<fframe_data> frame_data : register(b1);

fvs_output vs_main(fvs_input input)
{
  fvs_output output;
  output.position_cs    = mul(object_data.model_world_view_projection, float4(input.position, 1.0f));
  output.position_ws    = mul(object_data.model_world, float4(input.position, 1.0f));
  output.normal_ws      = normalize(mul((float3x3)object_data.inverse_transpose_model_world, input.normal));
  return output;
}

float4 ps_main(fvs_output input) : SV_Target
{
  const float3 V = normalize(frame_data.camera_position - input.position_ws).xyz;
  const float3 N = input.normal_ws;
  float factor = pow((1.0 - saturate(dot(N, V))), 2);  // Classic fresnel outline
  return float4(factor, factor, factor, 1);
}