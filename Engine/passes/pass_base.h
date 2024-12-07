#pragma once

#include <memory>
#include <wrl/client.h>

#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "renderer/graphics_pipeline.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  struct fgraphics_command_list;
  
  using Microsoft::WRL::ComPtr;
  
  struct frenderer_context;
  
  struct fpass_base
  {
    CTOR_DEFAULT(fpass_base)
    CTOR_MOVE_COPY_DELETE(fpass_base)
    VDTOR_DEFAULT(fpass_base)
    
    void init();
    virtual void draw(fgraphics_command_list* command_list);
    virtual void set_renderer_context(frenderer_context* in_context) { context = in_context; }
    bool get_can_draw() const { return can_draw; }
    
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;
    
  protected:
    virtual void init_pipeline();
    virtual void init_size_independent_resources() = 0;
    virtual void init_size_dependent_resources(bool cleanup) = 0;
    
    frenderer_context* context = nullptr; // weak ptr, owned by renderer
    std::unique_ptr<fgraphics_pipeline> graphics_pipeline;
    bool can_draw = true;
  };

}