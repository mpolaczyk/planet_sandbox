#pragma once

#include "renderer/async_renderer_base.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API gpu_renderer : public async_renderer_base
  {
  public:
    OBJECT_DECLARE(gpu_renderer, async_renderer_base)
  
  private:
    virtual void job_init() override;
    virtual void job_update() override;
    virtual void job_cleanup() override;
  };
}