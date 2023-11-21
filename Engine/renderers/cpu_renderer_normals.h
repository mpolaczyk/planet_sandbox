#pragma once

#include "renderer/cpu_renderer_base.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API cpu_renderer_normals : public cpu_renderer_base
  {
  public:
    OBJECT_DECLARE(cpu_renderer_normals, cpu_renderer_base)

    virtual std::string get_name() const override;

  private:
    virtual void render() override;

    void render_chunk(const chunk& in_chunk);
  };
}