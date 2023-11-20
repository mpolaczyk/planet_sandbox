#pragma once

#include "renderer/async_renderer_base.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API reference_renderer : public async_renderer_base
  {
  public:
    virtual std::string get_name() const override;

  private:
    virtual void render() override;

    void render_chunk(const chunk& in_chunk);

    vec3 fragment(float x, float y, const vec3& resolution);

    vec3 enviroment_light(const ray& in_ray);

    vec3 trace_ray(ray in_ray, uint32_t seed);
  };
}