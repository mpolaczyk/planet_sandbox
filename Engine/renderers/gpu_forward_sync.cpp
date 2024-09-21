﻿#include "object/object_registry.h"
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
    return forward_pass.get_can_render() && rrenderer_base::can_render();
  }
  
  void rgpu_forward_sync::init()
  {
    forward_pass.copy_input(window, &vertex_shader_asset.get()->render_state, &pixel_shader_asset.get()->render_state,
      &scene_acceleration, scene, selected_object,
      output_width, output_height,
      default_material_asset);

    forward_pass.init();
  }

  void rgpu_forward_sync::render_frame_internal(ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    forward_pass.copy_input(window, &vertex_shader_asset.get()->render_state, &pixel_shader_asset.get()->render_state,
      &scene_acceleration, scene, selected_object,
      output_width, output_height,
      default_material_asset);
    
    forward_pass.show_object_id = show_object_id;
    
    forward_pass.draw(command_list);
  };
}