#pragma once

#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
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

    // Persistent members
    fsoft_asset_ptr<apixel_shader> gbuffer_pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> gbuffer_vertex_shader_asset;
    fsoft_asset_ptr<apixel_shader> lighting_pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> lighting_vertex_shader_asset;

    // Runtime members
    fgbuffer_pass gbuffer_pass;
    fdeferred_lighting_pass deferred_lighting_pass;

    virtual ftexture_resource* get_color() override;
    virtual ftexture_resource* get_depth() override;

  protected:
    virtual bool can_draw() override;
    virtual void init() override;
    virtual void draw_internal(fgraphics_command_list* command_list) override;
  };
}