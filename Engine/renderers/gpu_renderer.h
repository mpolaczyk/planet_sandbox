#pragma once

#include "core/core.h"
#include "renderer/async_renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"

struct ID3D11InputLayout;
struct ID3D11Buffer;

namespace engine
{
  class ENGINE_API gpu_renderer : public async_renderer_base
  {
  public:
    OBJECT_DECLARE(gpu_renderer, async_renderer_base)
    
    // FIX make them persistent members
    soft_asset_ptr<pixel_shader_asset> pixel_shader;
    soft_asset_ptr<vertex_shader_asset> vertex_shader;

    virtual bool is_async() const override { return false; };
    
    ID3D11InputLayout* input_layout;
    ID3D11Buffer* vertex_buffer;
    unsigned int num_verts;
    unsigned int stride;
    unsigned int offset;
    
  private:
    virtual void job_init() override;
    virtual void job_update() override;
    virtual void job_cleanup() override;
  };
}
