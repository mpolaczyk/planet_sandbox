#pragma once

#include "renderer/cpu_renderer_base.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API cpu_renderer_preview : public cpu_renderer_base
  {
  public:
    OBJECT_DECLARE(cpu_renderer_preview, cpu_renderer_base)
  
  private:
    virtual void render() override;

    void render_chunk(const chunk& in_chunk);
  };
}