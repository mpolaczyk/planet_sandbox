#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_forward_sync.h"

#include "engine/log.h"

namespace engine
{
  OBJECT_DEFINE(rgpu_forward_sync, rrenderer_base, GPU forward sync)
  OBJECT_DEFINE_SPAWN(rgpu_forward_sync)
  OBJECT_DEFINE_VISITOR(rgpu_forward_sync)
  
  bool rgpu_forward_sync::can_render()
  {
    if(!rrenderer_base::can_render())
    {
      return false;
    }
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
    return true;
  }
  
  void rgpu_forward_sync::init()
  {
    forward_pass.vertex_shader = &vertex_shader_asset.get()->render_state;
    forward_pass.pixel_shader = &pixel_shader_asset.get()->render_state;
    forward_pass.scene_acceleration = &scene_acceleration;
    forward_pass.scene = scene;
    forward_pass.output_width = output_width;
    forward_pass.output_height = output_height;
    forward_pass.default_material_asset = default_material_asset;
    forward_pass.selected_object = selected_object;

    forward_pass.init();
  }

  void rgpu_forward_sync::render_frame_impl()
  {
    forward_pass.show_object_id = show_object_id;
    forward_pass.draw();
  };
}