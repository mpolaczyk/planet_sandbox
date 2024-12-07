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
  float4 position_cs : SV_Position;
  float2 uv	         : TEXCOORD;
};

struct fframe_data
{
  float4 camera_position;   // 16
  float4 ambient_light;     // 16
};

ConstantBuffer<fframe_data> frame_data : register(b0);
StructuredBuffer<flight_properties> lights_data : register(t0);
StructuredBuffer<fmaterial_properties> materials_data : register(t1);
Texture2D gbuffer_position : register(t2);
Texture2D gbuffer_normal : register(t3);
Texture2D gbuffer_uv : register(t4);
Texture2D gbuffer_material_id : register(t5);
Texture2D texture_data[] : register(t6);
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
  output.position_cs = float4(input.position, 1.0f);
  output.uv          = input.uv;
  return output;
}

float4 ps_main(fvs_output input) : SV_Target
{
  const float4 position = gbuffer_position.Sample(sampler_obj, input.uv);
  const float3 normal   = gbuffer_normal.Sample(sampler_obj, input.uv).xyz;
  const float2 uv       = gbuffer_uv.Sample(sampler_obj, input.uv).xy;
  const int material_id = gbuffer_material_id.Sample(sampler_obj, input.uv).x;
  
  const fmaterial_properties material = materials_data[material_id];
  const uint texture_id = NonUniformResourceIndex(material.texture_id);
    
  const flight_components light_final = compute_light(position, normal, material.specular_power);

  float4 tex_color = { 1, 1, 1, 1 };
  if (texture_id != -1)
  {
    tex_color = texture_data[texture_id].Sample(sampler_obj, uv);
  }

  float4 emissive = material.emissive;//  * frame_data.show_emissive;
  float4 ambient = material.ambient * frame_data.ambient_light;// * frame_data.show_ambient;
  float4 diffuse = material.diffuse * light_final.diffuse;// * frame_data.show_diffuse;
  float4 specular = material.specular * light_final.specular;// * frame_data.show_specular;
    
  return tex_color * (emissive + ambient + diffuse + specular);
}
