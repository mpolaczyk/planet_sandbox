#pragma once

#include "renderers/passes/pass_base.h"
#include "engine/renderer/gpu_resources.h"

namespace engine
{
  struct fcommand_list;
  
  struct fdebug_pass : public fpass_base
  {
    virtual void init_shaders() override;
    virtual void init_pipeline() override;
    virtual void init_size_independent_resources() override;
    virtual void init_size_dependent_resources(bool cleanup) override;
    virtual void draw(frenderer_context* in_context, fcommand_list* command_list) override;
    
    // Output
    ftexture_resource* blend_on = nullptr;

  private:
    std::vector<fconst_buffer> frame_data; // index is back buffer id
  };
  
};
