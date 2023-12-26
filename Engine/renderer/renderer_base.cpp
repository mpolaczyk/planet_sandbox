
#include <d3d11_1.h>

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
    if(output_texture != nullptr)
    {
      output_texture->Release();
      output_texture = nullptr;
    }
    if(output_srv != nullptr)
    {
      output_srv->Release();
      output_srv = nullptr;
    }
  }
 
}