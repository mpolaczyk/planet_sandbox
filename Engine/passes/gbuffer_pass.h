#pragma once

#include <wrl/client.h>

#include "dxcapi.h"

#include "passes/pass_base.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fgbuffer_pass : public fpass_base
  {
    virtual void init() override;
    virtual void draw(fgraphics_command_list* command_list) override;
    virtual void init_size_dependent(bool cleanup) override;

    // Outputs
    static constexpr uint32_t num_render_targets =  4;
    ftexture_resource* render_targets[num_render_targets];  // Helper, set at runtime
    ftexture_resource position;
    ftexture_resource normal;
    ftexture_resource uv;
    ftexture_resource material_id;
    ftexture_resource depth;
  };
}
