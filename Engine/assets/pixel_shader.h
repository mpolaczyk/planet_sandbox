#pragma once

#include "core/core.h"

#include "object/object.h"
#include "assets/shader.h"

namespace engine
{
  class ENGINE_API apixel_shader : public ashader
  {
  public:
    OBJECT_DECLARE(apixel_shader, ashader)

    virtual const char* get_extension() const override;
  };
}
