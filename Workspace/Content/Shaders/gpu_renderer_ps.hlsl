#include "gpu_renderer_common.hlsl"

#define POINT_LIGHT 0
#define DIRECTIONAL_LIGHT 1
#define SPOT_LIGHT 2

Texture2D    texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer fframe_data : register(b0)
{
    float4 camera_position;     // 16
    //
    float4 ambient_light;       // 16
    //
    flight_properties light;    // 80

    fmaterial_properties materials[MAX_MATERIALS];  // 80xN
};

cbuffer fobject_data : register(b1)
{
    matrix model_world;
    matrix inverse_transpose_model_world;
    matrix model_world_view_projection;
    int material_id;
};

struct flight_components
{
    float4 diffuse;
    float4 specular;
};

float4 get_light_diffuse(flight_properties light, float3 L, float3 N)
{
    const float NdotL = max(0, dot(N, L));
    return light.color * NdotL;
}

float4 get_light_specular(flight_properties light, float3 V, float3 L, float3 N)
{
    // Phong lighting.
    const float3 R = normalize(reflect(-L, N));
    const float RdotV = max(0, dot(R, V));

    // Blinn-Phong lighting
    const float3 H = normalize(L + V);
    const float NdotH = max(0, dot(N, H));

    const float material_specular_power = 1;
    return light.color * pow(RdotV, material_specular_power);
}

float get_light_attenuation(flight_properties light, float d)
{
    return 1.0f / ( light.constant_attenuation + light.linear_attenuation * d + light.quadratic_attenuation * d * d );
}

float get_light_cone_intensity(flight_properties light, float3 L)
{
    const float min_cos = cos(light.spot_angle);
    const float max_cos = (min_cos + 1.0f) / 2.0f;
    const float cos_angle = dot(light.direction.xyz, -L);
    return smoothstep(min_cos, max_cos, cos_angle); 
}


flight_components point_light(flight_properties light, float3 V, float4 P, float3 N)
{
    flight_components result;

    float3 L = (light.position - P).xyz;
    const float distance = length(L);
    L = L / distance;

    const float attenuation = get_light_attenuation(light, distance);

    result.diffuse = get_light_diffuse(light, L, N) * attenuation;
    result.specular = get_light_specular(light, V, L, N) * attenuation;

    return result;
}

flight_components directional_light(flight_properties light, float3 V, float4 P, float3 N)
{
    flight_components result;

    const float3 L = -light.direction.xyz;

    result.diffuse = get_light_diffuse(light, L, N);
    result.specular = get_light_specular(light, V, L, N);

    return result;
}

flight_components spot_light(flight_properties light, float3 V, float4 P, float3 N)
{
    flight_components result;

    float3 L = (light.position - P).xyz;
    const float distance = length(L);
    L = L / distance;

    const float attenuation = get_light_attenuation(light, distance);
    const float intensity = get_light_cone_intensity(light, L);

    result.diffuse = get_light_diffuse(light, L, N) * attenuation * intensity;
    result.specular = get_light_specular(light, V, L, N) * attenuation * intensity;

    return result;
}


flight_components compute_light(float4 P, float3 N)
{
    const float3 V = normalize(camera_position - P).xyz;

    flight_components final_light = { {0, 0, 0, 0}, {0, 0, 0, 0} };

    [unroll]
    for( int i = 0; i < 1; ++i )
    {
        flight_components delta_light = { {0, 0, 0, 0}, {0, 0, 0, 0} };
        
        switch(light.light_type)
        {
        case DIRECTIONAL_LIGHT:
            {
                delta_light = directional_light(light, V, P, N );
            }
            break;
        case POINT_LIGHT: 
            {
                delta_light = point_light(light, V, P, N );
            }
            break;
        case SPOT_LIGHT:
            {
                delta_light = spot_light(light, V, P, N );
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


float4 ps_main(VS_Output input) : SV_Target
{
    const flight_components light_final = compute_light(input.position_ws, input.normal_ws);

    const fmaterial_properties material = materials[material_id];
    return material.emissive
        + material.ambient * ambient_light
        + material.diffuse * light_final.diffuse
        + material.specular * light_final.specular;
    
    // Simple light test
    //const float4 light_dir = light.position - input.position_ws;
    //return ambient_light + light.color * saturate(dot(normalize(light_dir), input.normal_ws));

    // Draw normals
    //return float4(input.normal_ws * 0.5 + 0.5, 1);
    
    // Sample texture
    //return texture0.Sample(sampler0, input.uv);
}
