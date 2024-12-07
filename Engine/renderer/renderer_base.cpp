
#include "engine/log.h"
#include "engine/hash.h"
#include "hittables/scene.h"
#include "object/object_registry.h"
#include "profile/benchmark.h"
#include "renderer/renderer_base.h"

#include "core/application.h"
#include "renderer/command_list.h"

namespace engine
{
  OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
  OBJECT_DEFINE_NOSPAWN(rrenderer_base)
  
  void rrenderer_base::set_renderer_context(frenderer_context&& in_context)
  {
    if(!in_context.validate())
    {
      throw std::runtime_error("Failed to validate renderer context");
    }
    const bool resolution_changed = (context.width != in_context.width) || (context.height != in_context.height);
    context = std::move(in_context);
    context.resolution_changed = resolution_changed;
  }
  
  bool rrenderer_base::draw(fgraphics_command_list* command_list)
  {
    if(!can_draw())
    {
      return false;
    }
    
    // Initialize
    if(!init_done)
    {
      init();
      init_done = true;
    }
    
    context.scene->scene_acceleration.build(context.scene);
    if(!context.scene->scene_acceleration.validate())
    {
      return false;
    }

    draw_internal(command_list);
    return true;
  }

  bool rrenderer_base::can_draw()
  {
    if(MAX_TEXTURES + context.back_buffer_count * (MAX_MATERIALS + MAX_LIGHTS) > MAX_MAIN_DESCRIPTORS)
    {
      LOG_ERROR("Invalid main heap layout.");
      return false;
    }
    if(context.scene == nullptr)
    {
      LOG_ERROR("Can't draw. Scene is missing.");
      return false;
    }
    return true;
  }
}