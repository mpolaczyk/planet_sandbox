#pragma once


#include "app/factories.h"

class material
{
public:
  material() {}
  explicit material(material_type type) : type(type) 
  {
    if (type == material_type::light)
    {
      emitted_color = vec3(1.0f, 1.0f, 1.0f);
    }
  }
  material(std::string&& id, material_type type) : material(type)
  {
    id = std::move(id);
  }
 
  material_type type = material_type::none;
  vec3 color;
  vec3 emitted_color;
  vec3 gloss_color;
  vec3 pad;
  float smoothness = 0.0f;
  float gloss_probability = 0.0f;
  float refraction_probability = 0.0f;
  float refraction_index = 1.0f;

  std::string id;

  void get_name(std::string& out_name, bool with_params=true) const;
  void draw_edit_panel();  
};
