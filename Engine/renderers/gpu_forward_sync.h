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

    // Persistent members
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;

    // Runtime members
    fforward_pass forward_pass;
    
  protected:
    virtual bool can_draw() override;
    virtual void init() override;
    virtual void draw_internal(std::shared_ptr<fgraphics_command_list> command_list) override;
    virtual void create_output_texture(bool cleanup) override { forward_pass.create_output_texture(cleanup); };
  };
}
