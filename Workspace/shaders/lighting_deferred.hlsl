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
  uint width;               // 4
  uint height;              // 4
  uint2 pad;                // 8
};

// b
ConstantBuffer<fframe_data> frame_data : register(b0);
// t space0
StructuredBuffer<flight_properties> lights_data : register(t0, space0);
StructuredBuffer<fmaterial_properties> materials_data : register(t1, space0);
// t space1
Texture2D<float4> gbuffer_position : register(t0, space1);
Texture2D<float4> gbuffer_normal : register(t1, space1);
Texture2D<float2> gbuffer_uv : register(t2, space1);
Texture2D<uint> gbuffer_material_id : register(t3, space1);
// t space2
Texture2D texture_data[] : register(t0, space2);
// s
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
  const uint material_id = gbuffer_material_id.Load(int3(input.uv.x*frame_data.height, input.uv.y*frame_data.width, 0));

  const fmaterial_properties material = materials_data[NonUniformResourceIndex(material_id)];
  const uint texture_id = NonUniformResourceIndex(material.texture_id);

  const flight_components light_final = compute_light(position, normal, material.specular_power);

  float4 tex_color = { 1, 1, 1, 1 };
  if (texture_id != -1)
  {
    tex_color = texture_data[NonUniformResourceIndex(texture_id)].Sample(sampler_obj, uv);
  }

  float4 emissive = material.emissive;
  float4 ambient = material.ambient * frame_data.ambient_light;
  float4 diffuse = material.diffuse * light_final.diffuse;
  float4 specular = material.specular * light_final.specular;
    
  return tex_color * (emissive + ambient + diffuse + specular);
}
