#pragma once

#include "renderers/cpu_renderer.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API rcpu_faces : public rcpu
  {
  public:
    OBJECT_DECLARE(rcpu_faces, rcpu)
  
  private:
    virtual void worker_job_update() override;

    void render_chunk(const fchunk& in_chunk);
  };
}