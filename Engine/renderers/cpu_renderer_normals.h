#pragma once

#include "renderer/async_renderer_base.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API cpu_renderer_normals : public async_renderer_base
  {
  public:
    OBJECT_DECLARE(cpu_renderer_normals, async_renderer_base)
  
  private:
    virtual void job_update() override;

    void render_chunk(const chunk& in_chunk);
  };
}