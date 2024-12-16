#pragma once

#include "core/core.h"
#include "engine/renderer/renderer_base.h"
#include "passes/forward_pass.h"
#include "passes/debug_pass.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hstatic_mesh;
  class hlight;
  
  class ENGINE_API rforward : public rrenderer_base
  {
  public:
    OBJECT_DECLARE(rforward, rrenderer_base)
    OBJECT_DECLARE_VISITOR

    // Runtime members
    fforward_pass forward_pass;
    fdebug_pass debug_pass;

    virtual ftexture_resource* get_color() override;
    virtual ftexture_resource* get_depth() override;
    
  protected:
    virtual bool init_passes() override;
    virtual void draw_internal(fgraphics_command_list* command_list) override;
  };
}
