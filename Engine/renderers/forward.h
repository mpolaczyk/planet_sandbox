#pragma once

#include "core/core.h"
#include "engine/unique_ptr.h"

#include "engine/renderer/renderer_base.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hstatic_mesh;
  class hlight;
  struct fforward_pass;
  struct fdebug_pass;
  
  class ENGINE_API rforward : public rrenderer_base
  {
  public:
    CTOR_VDTOR(rforward)
    CTOR_MOVE_COPY_DELETE(rforward)

    OBJECT_DECLARE(rforward, rrenderer_base)
    OBJECT_DECLARE_VISITOR

    // Runtime members
    funique_ptr<fforward_pass> forward_pass;
    funique_ptr<fdebug_pass> debug_pass;

    virtual ftexture_resource* get_color() override;
    virtual ftexture_resource* get_depth() override;
    
  protected:
    virtual bool init_passes() override;
    virtual void draw_internal(fcommand_list* command_list) override;
  };
}
