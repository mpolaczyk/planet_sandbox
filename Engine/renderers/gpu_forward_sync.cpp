#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_forward_sync.h"

#include "engine/log.h"

namespace engine
{
  OBJECT_DEFINE(rgpu_forward_sync, rrenderer_base, GPU forward sync)
  OBJECT_DEFINE_SPAWN(rgpu_forward_sync)
  OBJECT_DEFINE_VISITOR(rgpu_forward_sync)
  
  bool rgpu_forward_sync::can_draw()
  {
    if(vertex_shader_asset.get() == nullptr)
    {
      vertex_shader_asset.set_name("forward");
      if(vertex_shader_asset.get() == nullptr)
      {
        LOG_ERROR("Missing vertex shader setup.");
        return false;
      }
    }
    if(pixel_shader_asset.get() == nullptr)
    {
      pixel_shader_asset.set_name("forward");
      if(pixel_shader_asset.get() == nullptr)
      {
        LOG_ERROR("Missing pixel shader setup.");
        return false;
      }
    }
    return forward_pass.get_can_draw() && rrenderer_base::can_draw();
  }
  
  void rgpu_forward_sync::init()
  {
    forward_pass.set_renderer_context(&context);
    forward_pass.vertex_shader_blob = vertex_shader_asset.get()->render_state.blob;
    forward_pass.pixel_shader_blob = pixel_shader_asset.get()->render_state.blob;
    forward_pass.init();
  }

  void rgpu_forward_sync::draw_internal(ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    forward_pass.set_renderer_context(&context);
    forward_pass.show_object_id = show_object_id; // TODO Selection should be done as a separate pass
    forward_pass.draw(command_list);
  };
}