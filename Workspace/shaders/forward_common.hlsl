
#define MAX_MATERIALS 32
#define MAX_LIGHTS 16

struct VS_Input
{
    float3 position  : POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv        : TEXCOORD;
};

struct VS_Output
{
    float4 position_ws  : TEXCOORD0;
    float3 normal_ws    : TEXCOORD1;
    float2 uv           : TEXCOORD2;
    float4 position     : SV_Position;
};

struct flight_properties
{
    float4 position;                // 16
    //
    float4 direction;               // 16
    //
    float4 color;                   // 16
    //
    float spot_angle;               // 4
    float constant_attenuation;     // 4
    float linear_attenuation;       // 4
    float quadratic_attenuation;    // 4
    //
    int light_type;                 // 4
    int enabled;                    // 4
    int2 padding;                   // 8
};

struct fmaterial_properties
{
    float4 emissive;        // 16
    //
    float4 ambient;         // 16
    //
    float4 diffuse;         // 16
    //
    float4 specular;        // 16
    //
    float specular_power;     // 4
    int use_texture;          // 4
    int2 padding;         // 8
};