#include "material_properties.hlsl"
#include "phong.hlsl"

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
  float4 position_cs : SV_POSITION;
  float2 uv	         : TEXCOORD;
};

Texture2D position_ws_texture		: register(t0);
Texture2D normal_ws_texture		  : register(t1);
Texture2D tex_color_texture		  : register(t2);
Texture2D<uint> material_id_texture		: register(t3);
Texture2D object_id_texture		        : register(t4);
Texture2D<uint> is_selected_texture		: register(t5);

sampler sampler0 : register(s0);

cbuffer fframe_data : register(b0)
{
  float4 camera_position;     // 16
  float4 ambient_light;       // 16
  matrix model_world_view_projection; // 64
  int show_position_ws;			// 4        // TODO bit flags
  int show_normal_ws;			  // 4
  int show_tex_color;			  // 4
  int show_object_id;        // 4
  flight_properties lights[MAX_LIGHTS];    // 80xN
  fmaterial_properties materials[MAX_MATERIALS];  // 80xN
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
  output.position_cs = float4(input.position, 1.0f);
  output.uv          = input.uv;
  return output;
}

float4 ps_main(fvs_output input) : SV_Target
{
  float4 position_ws   = position_ws_texture.Sample(sampler0, input.uv).xyzw;
  float3 normal_ws     = normal_ws_texture.Sample(sampler0, input.uv).xyz;
  float4 tex_color     = { 1, 1, 1, 1 };
  uint material_id     = material_id_texture.Load(int3(input.position_cs.xy, 0));
  float4 object_id     = object_id_texture.Sample(sampler0, input.uv).xyzw;
  uint is_selected     = is_selected_texture.Load(int3(input.position_cs.xy, 0));
  
  if(show_object_id)
  {
    return object_id;
  }
  if(show_position_ws)
  {
    return position_ws;
  }
  else if(show_normal_ws)
  {
    return float4(normal_ws * 0.5 + 0.5, 1);
  }
  else if(show_tex_color)
  {
    return float4(tex_color);
  }
  else
  {
    const fmaterial_properties material = materials[material_id];

    const flight_components light_final = compute_light(position_ws, normal_ws, material.specular_power);

    if (material.use_texture)
    {
      tex_color = tex_color_texture.Sample(sampler0, input.uv).xyzw;
    }
    
    const float4 selection_emissive = { 0.5, 0.5, 0.5, 1 };
    float4 emissive = max(material.emissive, is_selected * selection_emissive);
    float4 ambient = material.ambient * ambient_light;
    float4 diffuse = material.diffuse * light_final.diffuse;
    float4 specular = material.specular * light_final.specular;
 
    return tex_color * (emissive + ambient + diffuse + specular);
  }
}
