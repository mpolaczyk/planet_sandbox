#pragma once

#include "renderer/async_renderer_base.h"
#include "math/chunk_generator.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API example_renderer : public async_renderer_base
  {
  public:
    virtual std::string get_name() const override;

  private:
    virtual void render() override;

    void render_chunk(const chunk& in_chunk);
  };
}