#pragma once

#include "renderers/cpu_renderer.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API rcpu_preview : public rcpu
  {
  public:
      OBJECT_DECLARE(rcpu_preview, rcpu)
  
  private:
    virtual void job_update() override;

    void render_chunk(const fchunk& in_chunk);
  };
}