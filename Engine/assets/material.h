#pragma once

#include <string>

#include "core/core.h"

#include "object/object.h"
#include "math/vec3.h"
#include "asset/soft_asset_ptr.h"

namespace engine
{
  class ENGINE_API material_asset : public object
  {
  public:
    OBJECT_DECLARE(material_asset)

    virtual std::string get_display_name() const override;

    material_asset() = default;
    explicit material_asset(material_type type) : type(type)
    {
      if (type == material_type::light)
      {
        emitted_color = vec3(1.0f, 1.0f, 1.0f);
      }
    }
    material_asset(std::string&& id, material_type type) : material_asset(type)
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
  };
}
