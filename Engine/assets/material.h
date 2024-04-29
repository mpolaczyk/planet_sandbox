#pragma once

#include <DirectXMath.h>

#include "core/core.h"

#include "object/object.h"
#include "math/vec3.h"
#include "asset/asset.h"
#include "renderer/aligned_structs.h"

namespace engine
{
  using namespace DirectX;
  
  class ENGINE_API amaterial : public aasset_base
  {
  public:
    OBJECT_DECLARE(amaterial, aasset_base)
    OBJECT_DECLARE_LOAD(amaterial)
    OBJECT_DECLARE_SAVE(amaterial)
    OBJECT_DECLARE_VISITOR
    
    // JSON persistent - CPU Raytracer
    fvec3 color;
    fvec3 emitted_color;
    fvec3 gloss_color;
    fvec3 pad;
    float smoothness = 0.0f;
    float gloss_probability = 0.0f;
    float refraction_probability = 0.0f;
    float refraction_index = 1.0f;
    // JSON persistent - GPU renderer
    fmaterial_properties material;
  };
}
