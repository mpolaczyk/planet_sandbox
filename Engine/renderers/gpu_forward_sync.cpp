
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
  
  bool rgpu_forward_sync::can_draw()
  {
    return forward_pass.get_can_draw() && rrenderer_base::can_draw();
  }
  
  void rgpu_forward_sync::init()
  {
    forward_pass.set_renderer_context(&context);
    forward_pass.vertex_shader_asset = vertex_shader_asset;
    forward_pass.pixel_shader_asset = pixel_shader_asset;
    forward_pass.init();
  }

  void rgpu_forward_sync::draw_internal(fgraphics_command_list* command_list)
  {
    forward_pass.set_renderer_context(&context);
    forward_pass.show_object_id = show_object_id; // TODO Selection should be done as a separate pass
    forward_pass.draw(command_list);
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