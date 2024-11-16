
#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_deferred_sync.h"

#include "engine/log.h"

using namespace DirectX;

namespace engine
{
  OBJECT_DEFINE(rgpu_deferred_sync, rrenderer_base, GPU deferred sync)
  OBJECT_DEFINE_SPAWN(rgpu_deferred_sync)
  OBJECT_DEFINE_VISITOR(rgpu_deferred_sync)

  bool rgpu_deferred_sync::can_draw()
  {
    return deferred_lighting_pass.get_can_draw() && gbuffer_pass.get_can_draw() && rrenderer_base::can_draw();
  }

  void rgpu_deferred_sync::init()
  {
    gbuffer_pass.set_renderer_context(&context);
    gbuffer_pass.vertex_shader_asset = gbuffer_vertex_shader_asset;
    gbuffer_pass.pixel_shader_asset = gbuffer_pixel_shader_asset;
    gbuffer_pass.init();

    deferred_lighting_pass.set_renderer_context(&context);
    deferred_lighting_pass.vertex_shader_asset = lighting_vertex_shader_asset;
    deferred_lighting_pass.pixel_shader_asset = lighting_pixel_shader_asset;
    deferred_lighting_pass.init();
  }

  void rgpu_deferred_sync::draw_internal(fgraphics_command_list* command_list)
  {
    gbuffer_pass.set_renderer_context(&context);
    gbuffer_pass.show_object_id = show_object_id; // TODO Selection should be done as a separate pass
    gbuffer_pass.draw(command_list);
    
    //gbuffer_pass.set_renderer_context(&context);
    //
    //gbuffer_pass.init();
    //gbuffer_pass.draw(command_list);
    //
    //deferred_lighting_pass.set_renderer_context(&context);
    //
    //deferred_lighting_pass.init();
    //deferred_lighting_pass.gbuffer_srvs[egbuffer_type::material_id] = gbuffer_pass.output_srv[egbuffer_type::material_id];
    //deferred_lighting_pass.gbuffer_srvs[egbuffer_type::normal] = gbuffer_pass.output_srv[egbuffer_type::normal];
    //deferred_lighting_pass.gbuffer_srvs[egbuffer_type::position] = gbuffer_pass.output_srv[egbuffer_type::position];
    //deferred_lighting_pass.gbuffer_srvs[egbuffer_type::tex_color] = gbuffer_pass.output_srv[egbuffer_type::tex_color];
    //deferred_lighting_pass.gbuffer_srvs[egbuffer_type::object_id] = gbuffer_pass.output_srv[egbuffer_type::object_id];
    //deferred_lighting_pass.gbuffer_srvs[egbuffer_type::is_selected] = gbuffer_pass.output_srv[egbuffer_type::is_selected];
    //deferred_lighting_pass.show_object_id = show_object_id;
    //deferred_lighting_pass.draw(command_list);
  }
}