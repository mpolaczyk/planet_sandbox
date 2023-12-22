
#include "object/object_registry.h"
#include "renderers/gpu_renderer.h"

namespace engine
{
  OBJECT_DEFINE(gpu_renderer, cpu_renderer_base, GPU renderer)
  OBJECT_DEFINE_SPAWN(gpu_renderer)
  
  void gpu_renderer::render()
  {
    
  }
}