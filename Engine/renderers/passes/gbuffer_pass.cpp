#include "stdafx.h"

#include "gbuffer_pass.h"

#include "assets/material.h"
#include "assets/mesh.h"

#include "hittables/scene.h"
#include "hittables/static_mesh.h"

#include "engine/math/math.h"
#include "engine/math/vertex_data.h"

#include "engine/renderer/render_context.h"
#include "engine/renderer/command_list.h"
#include "engine/renderer/device.h"
#include "engine/renderer/scene_acceleration.h"
#include "engine/renderer/graphics_pipeline.h"

namespace engine
{
  using namespace DirectX;

  namespace
  {
    enum root_parameter_type : int
    {
      object_data = 0,
      num
    };
  
    DXGI_FORMAT rtv_formats[fgbuffer_pass::num_render_targets] = { DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32_UINT };
    const char* rtv_names[fgbuffer_pass::num_render_targets] = { "position", "normal", "uv", "material_id"};
  
    DXGI_FORMAT depth_format = DXGI_FORMAT_D32_FLOAT;
    const char* depth_name = "gbuffer pass depth";
  }

  void fgbuffer_pass::init_shaders()
  {
    pixel_shader_asset.set_name("gbuffer_deferred");
    vertex_shader_asset.set_name("gbuffer_deferred");
  }

  void fgbuffer_pass::init_pipeline()
  {
    fpass_base::init_pipeline();
    graphics_pipeline->reserve_parameters(root_parameter_type::num);
    graphics_pipeline->add_constant_parameter(root_parameter_type::object_data, 0, 0, fmath::to_uint32(sizeof(fobject_data)), D3D12_SHADER_VISIBILITY_PIXEL);
    graphics_pipeline->setup_formats(fgbuffer_pass::num_render_targets, rtv_formats, depth_format);
    graphics_pipeline->init("GBuffer pass");
  }

  void fgbuffer_pass::init_size_independent_resources()
  {
    render_targets[0] = &position;
    render_targets[1] = &normal;
    render_targets[2] = &uv;
    render_targets[3] = &material_id;
  }

  void fgbuffer_pass::init_size_dependent_resources(bool cleanup)
  {
    fdevice* device = fapplication::get_instance()->device.get();
    
    for(uint32_t i = 0; i < fgbuffer_pass::num_render_targets; i++)
    {
      render_targets[i]->release();
      device->create_frame_buffer(context->main_descriptor_heap, context->rtv_descriptor_heap, render_targets[i], context->width, context->height, rtv_formats[i], D3D12_RESOURCE_STATE_RENDER_TARGET, rtv_names[i]);
    }
    depth.release();
    device->create_depth_stencil(context->dsv_descriptor_heap, &depth, context->width, context->height, depth_format, D3D12_RESOURCE_STATE_DEPTH_WRITE, depth_name);
  }

  void fgbuffer_pass::draw(frenderer_context* in_context, fgraphics_command_list* command_list)
  {
    fpass_base::draw(in_context, command_list);
    
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();

    // Cleanup and setup
    const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for(uint32_t i = 0; i < fgbuffer_pass::num_render_targets; i++)
    {
      command_list->clear_render_target(render_targets[i], black);
    }
    command_list->clear_depth_stencil(&depth);
    command_list->set_render_targets(num_render_targets, render_targets, &depth);
    
    update_vertex_and_index_buffers(command_list);
    
    // Draw
    for(uint32_t i = 0; i < scene_acceleration.get_num_meshes(); i++)
    {
      const fstatic_mesh_resource& smrs = context->scene->scene_acceleration.get_mesh(i)->mesh_asset_ptr.get()->resource;

      command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::object_data, fmath::to_uint32(sizeof(fobject_data))/4, scene_acceleration.get_object_data(i), 0);
      command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
      command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
      command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
    }
  }
}