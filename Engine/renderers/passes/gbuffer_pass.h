#pragma once

#include "engine/renderer/gpu_resources.h"
#include "renderers/passes/pass_base.h"

namespace engine
{
  struct fgbuffer_pass : public fpass_base
  {
    virtual epipeline_type init_type() override { return epipeline_type::raster; }
    virtual void init_shaders() override;
    virtual void init_pipeline() override;
    virtual void init_size_independent_resources() override;
    virtual void init_size_dependent_resources(bool cleanup) override;
    virtual void draw(frenderer_context* in_context, fcommand_list* command_list) override;

    // Outputs
    static constexpr uint32_t num_render_targets =  4;
    ftexture_resource* render_targets[num_render_targets]{};  // Helper, set at runtime
    ftexture_resource position;
    ftexture_resource normal;
    ftexture_resource uv;
    ftexture_resource material_id;
    ftexture_resource depth;
  };
}
