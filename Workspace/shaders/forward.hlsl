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

//Texture2D texture0 : register(t0);
//sampler sampler0 : register(s0);

cbuffer fobject_data : register(b0)
{
  matrix model_world;
  matrix inverse_transpose_model_world;
  matrix model_world_view_projection;
  //float4 object_id;    
  uint material_id;
  //int is_selected;
};

cbuffer fframe_data : register(b1)
{
  float4 camera_position;     // 16
  float4 ambient_light;       // 16
//  int show_emissive;			    // 4        // TODO bit flags
//  int show_ambient;			      // 4
//  int show_specular;			    // 4
//  int show_diffuse; 			    // 4
//  int show_normals;			      // 4
//  int show_object_id;         // 4
//  int2 padding;				        // 8
  flight_properties lights[MAX_LIGHTS];             // 80xN
  fmaterial_properties materials[MAX_MATERIALS];    // 80xN
};

flight_components compute_light(float4 P, float3 N, float specular_power)
{
  const float3 V = normalize(camera_position - P).xyz;

  flight_components final_light = { {0, 0, 0, 0}, {0, 0, 0, 0} };

  [unroll]
  for( int i = 0; i < MAX_LIGHTS; ++i )
  {
    if(!lights[i].enabled)
      continue;
        
    flight_components delta_light = { {0, 0, 0, 0}, {0, 0, 0, 0} };
    switch(lights[i].light_type)
    {
    case DIRECTIONAL_LIGHT:
      {
        delta_light = directional_light(lights[i], V, P, N, specular_power);
      }
      break;
    case POINT_LIGHT: 
      {
        delta_light = point_light(lights[i], V, P, N, specular_power);
      }
      break;
    case SPOT_LIGHT:
      {
        delta_light = spot_light(lights[i], V, P, N, specular_power);
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
  output.position_cs    = mul(model_world_view_projection, float4(input.position, 1.0f));
  output.position_ws    = mul(model_world, float4(input.position, 1.0f));
  output.normal_ws      = normalize(mul((float3x3)inverse_transpose_model_world, input.normal));
  output.uv             = input.uv;
  return output;
}

float4 ps_main(fvs_output input) : SV_Target
{
  //return float4(input.normal_ws * ambient_light + 0.5, 1);
  //if(show_normals)
  //{
  //  return float4(input.normal_ws * 0.5 + 0.5, 1);
  //}
  //else if(show_object_id)
  //{
  //  return object_id;
  //}
  //else
  //{
  const fmaterial_properties material = materials[material_id];

  const flight_components light_final = compute_light(input.position_ws, input.normal_ws, material.specular_power);

  //float4 tex_color = { 1, 1, 1, 1 };
  //if (material.use_texture)
  //{
  //  tex_color = texture0.Sample(sampler0, input.uv);
  //}
  
  //const float4 selection_emissive = { 0.5, 0.5, 0.5, 1 };
  //float4 emissive = max(material.emissive, is_selected * selection_emissive);
  float4 emissive = material.emissive;
  float4 ambient = material.ambient * ambient_light;
  float4 diffuse = material.diffuse * light_final.diffuse;
  float4 specular = material.specular * light_final.specular;
 
  return emissive + ambient + diffuse + specular;
}