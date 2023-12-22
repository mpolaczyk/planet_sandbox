#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"

namespace engine
{
  class ENGINE_API pixel_shader_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(pixel_shader_asset, asset_base)
    OBJECT_DECLARE_LOAD(pixel_shader_asset)
    OBJECT_DECLARE_SAVE(pixel_shader_asset)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string shader_file_name;
  };
}