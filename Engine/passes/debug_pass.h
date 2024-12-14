#pragma once

#include <wrl/client.h>

#include "passes/pass_base.h"
#include "renderer/gpu_resources.h"

namespace engine
{
  struct fgraphics_command_list;
  using Microsoft::WRL::ComPtr;
  
  struct fdebug_pass : public fpass_base
  {
    virtual void init_shaders() override;
    virtual void init_pipeline() override;
    virtual void init_size_independent_resources() override;
    virtual void init_size_dependent_resources(bool cleanup) override;
    virtual void draw(frenderer_context* in_context, fgraphics_command_list* command_list) override;
    
    // Output
    ftexture_resource* blend_on;

  private:
    std::vector<fconst_buffer> frame_data; // index is back buffer id
  };
  
};