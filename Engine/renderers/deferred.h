#pragma once

#include "core/core.h"
#include "engine/unique_ptr.h"

#include "engine/renderer/renderer_base.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hstatic_mesh;
  class hlight;
  struct fgbuffer_pass;
  struct fdeferred_lighting_pass;
  struct fdebug_pass;

  class ENGINE_API rdeferred : public rrenderer_base
  {
  public:
    CTOR_VDTOR(rdeferred)
    CTOR_MOVE_COPY_DELETE(rdeferred)

    OBJECT_DECLARE(rdeferred, rrenderer_base)
    OBJECT_DECLARE_VISITOR

    // Runtime members
    funique_ptr<fgbuffer_pass> gbuffer_pass;
    funique_ptr<fdeferred_lighting_pass> deferred_lighting_pass;
    funique_ptr<fdebug_pass> debug_pass;

    virtual ftexture_resource* get_color() override;
    virtual ftexture_resource* get_depth() override;

  protected:
    virtual bool init_passes() override;
    virtual void draw_internal(fgraphics_command_list* command_list) override;
  };
}