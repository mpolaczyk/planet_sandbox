#include "phong.hlsl"
#include "material_properties.hlsl"

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16

#define POINT_LIGHT 0
#define DIRECTIONAL_LIGHT 1
#define SPOT_LIGHT 2

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
  float4 ambient_light;       // 16
  uint width;                 // 4
  uint height;                // 4
  uint2 pad;                  // 8
};

ConstantBuffer<fobject_data> object_data : register(b0);
ConstantBuffer<fframe_data> frame_data : register(b1);
StructuredBuffer<flight_properties> lights_data : register(t0);
StructuredBuffer<fmaterial_properties> materials_data : register(t1);
Texture2D texture_data[] : register(t2);
SamplerState sampler_obj : register(s0);


flight_components compute_light(float4 P, float3 N, float specular_power)
{
  const float3 V = normalize(frame_data.camera_position - P).xyz;

  flight_components final_light = { {0, 0, 0, 0}, {0, 0, 0, 0} };
  
  [unroll]
  for( int i = 0; i < MAX_LIGHTS; ++i )
  {
    const flight_properties light = lights_data[i];
    if(!light.enabled)
      continue;
        
    flight_components delta_light = { {0, 0, 0, 0}, {0, 0, 0, 0} };
    switch(light.light_type)
    {
    case DIRECTIONAL_LIGHT:
      {
        delta_light = directional_light(light, V, P, N, specular_power);
      }
      break;
    case POINT_LIGHT: 
      {
        delta_light = point_light(light, V, P, N, specular_power);
      }
      break;
    case SPOT_LIGHT:
      {
        delta_light = spot_light(light, V, P, N, specular_power);
      }
      break;
    }
    final_light.diffuse += delta_light.diffuse;
    final_light.specular += delta_light.specular;
  }

  final_light.diffuse = saturate(final_light.diffuse);
  final_light.specular = saturate(final_light.specular);

  return final_light;
}

fvs_output vs_main(fvs_input input)
{
  fvs_output output;
  output.position_cs    = mul(object_data.model_world_view_projection, float4(input.position, 1.0f));
  output.position_ws    = mul(object_data.model_world, float4(input.position, 1.0f));
  output.normal_ws      = normalize(mul((float3x3)object_data.inverse_transpose_model_world, input.normal));
  output.uv             = input.uv;
  return output;
}

float4 ps_main(fvs_output input) : SV_Target
{
  const uint material_id = NonUniformResourceIndex(object_data.material_id);
  const fmaterial_properties material = materials_data[material_id];
  const int texture_id = NonUniformResourceIndex(material.texture_id);

  const flight_components light_final = compute_light(input.position_ws, input.normal_ws, material.specular_power);
 
  float4 tex_color = { 1, 1, 1, 1 };
  if (texture_id != -1)
  {
    tex_color = texture_data[texture_id].Sample(sampler_obj, input.uv);
  }
  
  float4 emissive = material.emissive;
  float4 ambient = material.ambient * frame_data.ambient_light;
  float4 diffuse = material.diffuse * light_final.diffuse;
  float4 specular = material.specular * light_final.specular;
  
  return tex_color * (emissive + ambient + diffuse + specular);
}