#pragma once

#include "core/core.h"

#include "object/object.h"
#include "math/vec3.h"
#include "asset/asset.h"

namespace engine
{
  class ENGINE_API texture_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(texture_asset, asset_base)
    OBJECT_DECLARE_LOAD(texture_asset)
    OBJECT_DECLARE_SAVE(texture_asset)

    virtual std::string get_display_name() const override;

    // JSON persistent
    std::string img_file_name;
    int width;
    int height;

    // Image file data
    bool is_hdr;
    uint8_t* data_ldr = nullptr;
    float* data_hdr = nullptr;
  };
}