
#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "forward.h"

#include "engine/log.h"
#include "renderer/command_list.h"

namespace engine
{
  OBJECT_DEFINE(rforward, rrenderer_base, Forward renderer)
  OBJECT_DEFINE_SPAWN(rforward)
  OBJECT_DEFINE_VISITOR(rforward)
  
  bool rforward::init_passes()
  {
    return forward_pass.init(&context) && debug_pass.init(&context);
  }

  void rforward::draw_internal(fgraphics_command_list* command_list)
  {
    forward_pass.draw(&context, command_list);

    debug_pass.blend_on = &forward_pass.color;
    
    debug_pass.draw(&context, command_list);
  }

  ftexture_resource* rforward::get_color()
  {
    return &forward_pass.color;
  }

  ftexture_resource* rforward::get_depth()
  {
    return &forward_pass.depth;
  }
}