#pragma once

#include "core/core.h"

#include "assets/shader.h"

namespace engine
{
  class ENGINE_API avertex_shader : public ashader
  {
  public:
    OBJECT_DECLARE(avertex_shader, ashader)

    virtual std::string get_extension() const override;
  };
}
