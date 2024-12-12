#pragma once

#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "passes/debug_pass.h"
#include "passes/deferred_lighting_pass.h"
#include "passes/gbuffer_pass.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hstatic_mesh;
  class hlight;
  
  using Microsoft::WRL::ComPtr;

  class ENGINE_API rgpu_deferred_sync : public rrenderer_base
  {
  public:
    OBJECT_DECLARE(rgpu_deferred_sync, rrenderer_base)
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