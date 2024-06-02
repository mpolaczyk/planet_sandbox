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