#pragma once

#include "core/core.h"

#include "object/object.h"
#include "math/vec3.h"
#include "asset/asset.h"

namespace engine
{
  class ENGINE_API material_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(material_asset, asset_base)
    OBJECT_DECLARE_LOAD(material_asset)
    OBJECT_DECLARE_SAVE(material_asset)
    OBJECT_DECLARE_VISITOR
    
    // JSON persistent
    bool is_light = false;
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
