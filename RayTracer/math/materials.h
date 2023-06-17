#pragma once

#include "app/asset.h"
#include "app/factories.h"

class material : public asset
{
public:
  static asset_type get_static_asset_type();
  static material* load(const std::string& material_name);
  static void save(material* object);
  static material* spawn();

  virtual std::string get_display_name() const override;

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
 
  // JSON persistent
  material_type type = material_type::none;
  vec3 color;
  vec3 emitted_color;
  vec3 gloss_color;
  vec3 pad;
  float smoothness = 0.0f;
  float gloss_probability = 0.0f;
  float refraction_probability = 0.0f;
  float refraction_index = 1.0f;

  void draw_edit_panel();
};
