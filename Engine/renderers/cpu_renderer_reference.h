#pragma once

#include "renderers/cpu_renderer.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API rcpu_reference : public rcpu
  {
  public:
    OBJECT_DECLARE(rcpu_reference, rcpu)
  
  private:
    virtual void job_update() override;

    void render_chunk(const fchunk& in_chunk);

    fvec3 fragment(float x, float y, const fvec3& resolution);

    fvec3 enviroment_light(const fray& in_ray);

    fvec3 trace_ray(fray in_ray, uint32_t seed);
  };
}