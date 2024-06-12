﻿#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_deferred_sync.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/scene.h"

using namespace DirectX;

namespace engine
{
  OBJECT_DEFINE(rgpu_deferred_sync, rrenderer_base, GPU deferred sync)
  OBJECT_DEFINE_SPAWN(rgpu_deferred_sync)
  OBJECT_DEFINE_VISITOR(rgpu_deferred_sync)

  bool rgpu_deferred_sync::can_render()
  {
    if(!rrenderer_base::can_render())
    {
      return false;
    }
    if(gbuffer_vertex_shader_asset.get() == nullptr)
    {
      gbuffer_vertex_shader_asset.set_name("gbuffer_deferred");
      if(gbuffer_vertex_shader_asset.get() == nullptr)
      {
        LOG_ERROR("Missing gbuffer vertex shader setup.");
        return false;
      }
    }
    if(gbuffer_pixel_shader_asset.get() == nullptr)
    {
      gbuffer_pixel_shader_asset.set_name("gbuffer_deferred");
      if(gbuffer_pixel_shader_asset.get() == nullptr)
      {
        LOG_ERROR("Missing gbuffer pixel shader setup.");
        return false;
      }
    }
    if(lighting_vertex_shader_asset.get() == nullptr)
    {
      lighting_vertex_shader_asset.set_name("lighting_deferred");
      if(lighting_vertex_shader_asset.get() == nullptr)
      {
        LOG_ERROR("Missing lighting vertex shader setup.");
        return false;
      }
    }
    if(lighting_pixel_shader_asset.get() == nullptr)
    {
      lighting_pixel_shader_asset.set_name("lighting_deferred");
      if(lighting_pixel_shader_asset.get() == nullptr)
      {
        LOG_ERROR("Missing lighting pixel shader setup.");
        return false;
      }
    }
    return true;
  }

  void rgpu_deferred_sync::init()
  {
    auto dx = fdx11::instance();

    // TODO Function call
    gbuffer_pass.vertex_shader = &gbuffer_vertex_shader_asset.get()->render_state;
    gbuffer_pass.pixel_shader = &gbuffer_pixel_shader_asset.get()->render_state;
    gbuffer_pass.scene_acceleration = &scene_acceleration;
    gbuffer_pass.scene = scene;
    gbuffer_pass.output_width = output_width;
    gbuffer_pass.output_height = output_height;
    gbuffer_pass.default_material_asset = default_material_asset;

    // TODO Function call
    deferred_lighting_pass.vertex_shader = &lighting_vertex_shader_asset.get()->render_state;
    deferred_lighting_pass.pixel_shader = &lighting_pixel_shader_asset.get()->render_state;
    deferred_lighting_pass.scene_acceleration = &scene_acceleration;
    deferred_lighting_pass.scene = scene;
    deferred_lighting_pass.output_width = output_width;
    deferred_lighting_pass.output_height = output_height;
    deferred_lighting_pass.default_material_asset = default_material_asset;
  }

  void rgpu_deferred_sync::render_frame_impl()
  {
    gbuffer_pass.init();
    gbuffer_pass.selected_object = selected_object;
    gbuffer_pass.draw();
    
    deferred_lighting_pass.init();
    deferred_lighting_pass.selected_object = selected_object;
    deferred_lighting_pass.gbuffer_srvs[egbuffer_type::material_id] = gbuffer_pass.output_srv[egbuffer_type::material_id];
    deferred_lighting_pass.gbuffer_srvs[egbuffer_type::normal] = gbuffer_pass.output_srv[egbuffer_type::normal];
    deferred_lighting_pass.gbuffer_srvs[egbuffer_type::position] = gbuffer_pass.output_srv[egbuffer_type::position];
    deferred_lighting_pass.gbuffer_srvs[egbuffer_type::tex_color] = gbuffer_pass.output_srv[egbuffer_type::tex_color];
    deferred_lighting_pass.gbuffer_srvs[egbuffer_type::object_id] = gbuffer_pass.output_srv[egbuffer_type::object_id];
    deferred_lighting_pass.gbuffer_srvs[egbuffer_type::is_selected] = gbuffer_pass.output_srv[egbuffer_type::is_selected];
    deferred_lighting_pass.show_object_id = show_object_id;
    deferred_lighting_pass.draw();
    
  }

  void rgpu_deferred_sync::create_output_texture(bool cleanup)
  {
    gbuffer_pass.create_output_texture(cleanup);
    deferred_lighting_pass.create_output_texture(cleanup);
  };
}