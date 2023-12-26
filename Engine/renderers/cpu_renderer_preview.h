#pragma once

#include "renderer/renderer_base.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API cpu_renderer_preview : public renderer_base
  {
  public:
      OBJECT_DECLARE(cpu_renderer_preview, renderer_base)
  
  private:
    virtual void job_update() override;

    void render_chunk(const chunk& in_chunk);
  };
}