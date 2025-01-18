#pragma once

#include "core/core.h"

#include "assets/shader.h"

namespace engine
{
  class ENGINE_API aray_tracing_shader : public ashader
  {
  public:
    OBJECT_DECLARE(aray_tracing_shader, ashader)

    virtual std::string get_extension() const override;
  };
}