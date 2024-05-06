#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"
#include "renderer/render_state.h"

namespace engine
{
  class ENGINE_API atexture : public aasset_base
  {
  public:
    OBJECT_DECLARE(atexture, aasset_base)
    OBJECT_DECLARE_LOAD(atexture)
    OBJECT_DECLARE_SAVE(atexture)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string img_file_name;
    int desired_channels;
    
    // Image file data
    int width;
    int height;
    ftexture_render_state render_state;
  };
}