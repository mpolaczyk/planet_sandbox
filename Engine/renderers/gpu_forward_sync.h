#pragma once

#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "passes/forward_pass.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hstatic_mesh;
  class hlight;

  using Microsoft::WRL::ComPtr;
  
  class ENGINE_API rgpu_forward_sync : public rrenderer_base
  {
  public:
    OBJECT_DECLARE(rgpu_forward_sync, rrenderer_base)
    OBJECT_DECLARE_VISITOR

    // Runtime members
    fforward_pass forward_pass;

    virtual ftexture_resource* get_color() override;
    virtual ftexture_resource* get_depth() override;
    
  protected:
    virtual bool init_passes() override;
    virtual void draw_internal(fgraphics_command_list* command_list) override;
  };
}
