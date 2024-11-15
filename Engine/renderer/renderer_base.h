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
    int show_object_id = 0;
    
    // Persistent
    int output_width = 1920;
    int output_height = 1080;

    // Main public interface
    void set_renderer_context(frenderer_context&& in_context);
    void draw(fgraphics_command_list* command_list);
    
  protected:
    virtual bool can_draw();
    virtual void init() = 0;
    virtual void draw_internal(fgraphics_command_list* command_list) = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
  
  private:
    uint32_t last_frame_resolution_hash = 0;
    bool init_done = false;
  };
}
