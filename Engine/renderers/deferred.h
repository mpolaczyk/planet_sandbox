#pragma once

#include "core/core.h"
#include "core/com_pointer.h"
#include "engine/renderer/renderer_base.h"
#include "passes/debug_pass.h"
#include "passes/deferred_lighting_pass.h"
#include "passes/gbuffer_pass.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hstatic_mesh;
  class hlight;
  
  class ENGINE_API rdeferred : public rrenderer_base
  {
  public:
    OBJECT_DECLARE(rdeferred, rrenderer_base)
    OBJECT_DECLARE_VISITOR

    // Runtime members
    fgbuffer_pass gbuffer_pass;
    fdeferred_lighting_pass deferred_lighting_pass;
    fdebug_pass debug_pass;

    virtual ftexture_resource* get_color() override;
    virtual ftexture_resource* get_depth() override;

  protected:
    virtual bool init_passes() override;
    virtual void draw_internal(fgraphics_command_list* command_list) override;
  };
}