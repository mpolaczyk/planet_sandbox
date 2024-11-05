#pragma once

#include <wrl/client.h>

#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "renderer/graphics_pipeline.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct frenderer_context;
  
  struct fpass_base
  {
    virtual ~fpass_base() = default;
    
    virtual void init();
    virtual void draw(ComPtr<ID3D12GraphicsCommandList> command_list) = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
    virtual void set_renderer_context(frenderer_context* in_context)
    {
      context = in_context;
    }
    bool get_can_draw() const { return can_draw; }
    
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;
    
  protected:
    frenderer_context* context = nullptr; // weak ptr, owned by renderer
    fgraphics_pipeline graphics_pipeline;
    bool can_draw = true;
  };

}