#pragma once

#include "renderer/cpu_renderer_base.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API gpu_renderer : public cpu_renderer_base
  {
  public:
    OBJECT_DECLARE(gpu_renderer, cpu_renderer_base)
  
  private:
    virtual void render() override;
  };
}