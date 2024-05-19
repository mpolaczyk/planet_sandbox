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