#pragma once

#include "core/rtti/object.h"
#include "engine/renderer/render_context.h"

namespace engine
{
  struct fshader_resource;
  struct ftexture_resource;
  struct fcommand_list;
  
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
    bool draw(frenderer_context&& in_context, fcommand_list* command_list);
    virtual ftexture_resource* get_color() = 0;
    virtual ftexture_resource* get_depth() = 0;
    
  protected:
    bool can_draw() const;
    virtual bool init_passes() = 0;
    virtual void draw_internal(fcommand_list* command_list) = 0;
  
  private:
    void set_renderer_context(frenderer_context&& in_context);
    bool init_done = false;
  };
}
