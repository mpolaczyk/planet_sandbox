
#include <d3d11_1.h>

#include "renderer/dx11_lib.h"

#include "renderer/renderer_base.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(renderer_base, object, Renderer base)
  OBJECT_DEFINE_NOSPAWN(renderer_base)

  renderer_base::~renderer_base()
  {
    cleanup();
  }
  
  void renderer_base::cleanup()
  {
    DX_RELEASE(output_texture)
    DX_RELEASE(output_srv)
  }
 
}