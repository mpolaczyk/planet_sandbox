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
    
    // JSON persistent
    fmaterial_properties properties;
  };
}
