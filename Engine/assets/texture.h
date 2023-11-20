#pragma once

#include "core/core.h"

#include "object/object.h"
#include "math/vec3.h"

namespace engine
{
  class ENGINE_API texture_asset : public object
  {
  public:
    OBJECT_DECLARE(texture_asset)

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