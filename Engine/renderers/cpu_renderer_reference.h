#pragma once

#include "renderers/cpu_renderer.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API cpu_renderer_reference : public cpu_renderer
  {
  public:
    OBJECT_DECLARE(cpu_renderer_reference, cpu_renderer)
  
  private:
    virtual void job_update() override;

    void render_chunk(const chunk& in_chunk);

    vec3 fragment(float x, float y, const vec3& resolution);

    vec3 enviroment_light(const ray& in_ray);

    vec3 trace_ray(ray in_ray, uint32_t seed);
  };
}