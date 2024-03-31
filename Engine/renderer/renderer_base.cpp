
#include <d3d11_1.h>

#include "renderer/dx11_lib.h"

#include "renderer/renderer_base.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
  OBJECT_DEFINE_NOSPAWN(rrenderer_base)
  
  void rrenderer_base::destroy()
  {
    DX_RELEASE(output_texture)
    DX_RELEASE(output_srv)
    DX_RELEASE(output_depth_texture)
    oobject::destroy();
  }
 
}