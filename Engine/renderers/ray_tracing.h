#pragma once

#include "core/core.h"
#include "engine/unique_ptr.h"

#include "engine/renderer/renderer_base.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hstatic_mesh;
  class hlight;
  struct fray_tracing_pass;
  
  class ENGINE_API rray_tracing : public rrenderer_base
  {
  public:
    CTOR_VDTOR(rray_tracing)
    CTOR_MOVE_COPY_DELETE(rray_tracing)

    OBJECT_DECLARE(rray_tracing, rrenderer_base)
    OBJECT_DECLARE_VISITOR

    // Runtime members
    funique_ptr<fray_tracing_pass> ray_tracing_pass;

    virtual ftexture_resource* get_color() override;
    virtual ftexture_resource* get_depth() override;
    
  protected:
    virtual bool init_passes() override;
    virtual void draw_internal(fcommand_list* command_list) override;
  };
}
