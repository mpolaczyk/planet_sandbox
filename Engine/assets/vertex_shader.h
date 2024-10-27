#pragma once

#include "core/core.h"

#include "object/object.h"
#include "assets/shader.h"

namespace engine
{
  class ENGINE_API avertex_shader : public ashader
  {
  public:
    OBJECT_DECLARE(avertex_shader, ashader)

    virtual const char* get_extension() override
    {
      return ".vertex_shader";
    }
  };
}
