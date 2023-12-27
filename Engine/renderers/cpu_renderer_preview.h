#pragma once

#include "renderers/cpu_renderer.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API cpu_renderer_preview : public cpu_renderer
  {
  public:
      OBJECT_DECLARE(cpu_renderer_preview, cpu_renderer)
  
  private:
    virtual void job_update() override;

    void render_chunk(const chunk& in_chunk);
  };
}