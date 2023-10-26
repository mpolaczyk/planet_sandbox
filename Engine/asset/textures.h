#pragma once

#include "core/core.h"

#include "asset/asset.h"
#include "math/vec3.h"

namespace engine
{
  class ENGINE_API texture : public asset
  {
  public:
    static asset_type get_static_asset_type();
    static texture* load(const std::string& texture_name);
    static void save(texture* object);
    static texture* spawn();

    virtual std::string get_display_name() const override;

    vec3 value(float u, float v, const vec3& p) const
    {
      return vec3(0, 0, 0);
    }

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