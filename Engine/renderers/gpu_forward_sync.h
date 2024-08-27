#pragma once

#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "passes/forward_pass.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"

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

    fforward_pass forward_pass;
    
  protected:
    virtual bool can_render() override;
    virtual void init() override;
    virtual void render_frame_internal(ComPtr<ID3D12GraphicsCommandList> command_list) override;
    virtual void create_output_texture(bool cleanup) override { forward_pass.create_output_texture(cleanup); };
  };
}
