#pragma once

#include <string>
#include <wrl/client.h>

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "object/object.h"
#include "renderer/render_context.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hscene;
  class fwindow;
  struct fshader_resource;
  struct fgraphics_command_list;
  
  // The responsibility of this class is to render to a texture
  class ENGINE_API rrenderer_base : public oobject
  {
  public:
    OBJECT_DECLARE(rrenderer_base, oobject)

    CTOR_DEFAULT(rrenderer_base)
    CTOR_MOVE_COPY_DELETE(rrenderer_base)
    VDTOR_DEFAULT(rrenderer_base)

    // Runtime
    frenderer_context context;
    
    // Main public interface
    void set_renderer_context(frenderer_context&& in_context);
    bool draw(fgraphics_command_list* command_list);
    virtual ftexture_resource* get_color() = 0;
    virtual ftexture_resource* get_depth() = 0;
    
  protected:
    virtual bool can_draw();
    virtual void init() = 0;
    virtual void draw_internal(fgraphics_command_list* command_list) = 0;
  
  private:
    bool init_done = false;
  };
}
