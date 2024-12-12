
#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_forward_sync.h"

#include "engine/log.h"
#include "renderer/command_list.h"

namespace engine
{
  OBJECT_DEFINE(rgpu_forward_sync, rrenderer_base, GPU forward sync)
  OBJECT_DEFINE_SPAWN(rgpu_forward_sync)
  OBJECT_DEFINE_VISITOR(rgpu_forward_sync)
  
  bool rgpu_forward_sync::init_passes()
  {
    return forward_pass.init(&context) && debug_pass.init(&context);
  }

  void rgpu_forward_sync::draw_internal(fgraphics_command_list* command_list)
  {
    forward_pass.draw(&context, command_list);

    debug_pass.blend_on = &forward_pass.color;
    
    debug_pass.draw(&context, command_list);
  }

  ftexture_resource* rgpu_forward_sync::get_color()
  {
    return &forward_pass.color;
  }

  ftexture_resource* rgpu_forward_sync::get_depth()
  {
    return &forward_pass.depth;
  }
}