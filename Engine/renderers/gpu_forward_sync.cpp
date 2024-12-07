﻿
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
    return vertex_shader_asset.is_loaded() && vertex_shader_asset.get()->compilation_successful
      && pixel_shader_asset.is_loaded() && pixel_shader_asset.get()->compilation_successful
      && rrenderer_base::can_draw(); 
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
    fresource_barrier_scope a(command_list, forward_pass.color.com.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    fresource_barrier_scope b(command_list, forward_pass.depth.com.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    forward_pass.set_renderer_context(&context);
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