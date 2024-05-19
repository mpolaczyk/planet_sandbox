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

float4 get_light_specular(flight_properties light, float3 V, float3 L, float3 N, float specular_power)
{
    // Phong lighting.
    const float3 R = normalize(reflect(-L, N));
    const float RdotV = max(0, dot(R, V));

    // Blinn-Phong lighting
    const float3 H = normalize(L + V);
    const float NdotH = max(0, dot(N, H));

    return light.color * pow(RdotV, specular_power);
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

flight_components point_light(flight_properties light, float3 V, float4 P, float3 N, float specular_power)
{
    flight_components result;

    float3 L = (light.position - P).xyz;
    const float distance = length(L);
    L = L / distance;

    const float attenuation = get_light_attenuation(light, distance);

    result.diffuse = get_light_diffuse(light, L, N) * attenuation;
    result.specular = get_light_specular(light, V, L, N, specular_power) * attenuation;

    return result;
}

flight_components directional_light(flight_properties light, float3 V, float4 P, float3 N, float specular_power)
{
    flight_components result;

    const float3 L = -light.direction.xyz;

    result.diffuse = get_light_diffuse(light, L, N);
    result.specular = get_light_specular(light, V, L, N, specular_power);

    return result;
}

flight_components spot_light(flight_properties light, float3 V, float4 P, float3 N, float specular_power)
{
    flight_components result;

    float3 L = (light.position - P).xyz;
    const float distance = length(L);
    L = L / distance;

    const float attenuation = get_light_attenuation(light, distance);
    const float intensity = get_light_cone_intensity(light, L);

    result.diffuse = get_light_diffuse(light, L, N) * attenuation * intensity;
    result.specular = get_light_specular(light, V, L, N, specular_power) * attenuation * intensity;

    return result;
}