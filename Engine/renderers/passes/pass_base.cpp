
#include "renderers/passes/pass_base.h"

#include "assets/texture.h"
#include "core/application.h"
#include "engine/window.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "engine/math/math.h"
#include "engine/math/vertex_data.h"
#include "engine/renderer/command_list.h"
#include "engine/renderer/device.h"
#include "engine/renderer/render_context.h"

namespace engine
{
  bool fpass_base::init(frenderer_context* in_context)
  {
    context = in_context;

    // Get shader names, load and compile them, make sure they are valid
    init_shaders();
    pixel_shader_asset.get();
    vertex_shader_asset.get();
    if(!can_draw())
    {
      return false;
    }

    // Continue with the rest of initialization
    init_pipeline();
    init_size_independent_resources();
    init_size_dependent_resources(false);
    return true;
  }

  void fpass_base::init_pipeline()
  {
    if(graphics_pipeline)
    {
      graphics_pipeline.reset(nullptr);
    }
    graphics_pipeline = std::make_unique<fgraphics_pipeline>();
    graphics_pipeline->bind_pixel_shader(pixel_shader_asset.get()->resource.blob);
    graphics_pipeline->bind_vertex_shader(vertex_shader_asset.get()->resource.blob);
    graphics_pipeline->setup_input_layout(fvertex_data::input_layout);
  }

  void fpass_base::draw(frenderer_context* in_context, fgraphics_command_list* command_list)
  {
    context = in_context;

    // Handle shaders hotswap
    if(pixel_shader_asset.get()->hot_swap_requested || vertex_shader_asset.get()->hot_swap_requested)
    {
      LOG_INFO("Recreating pipeline state.")
      graphics_pipeline.reset(nullptr);
      init_pipeline();
      pixel_shader_asset.get()->hot_swap_done = true;
      vertex_shader_asset.get()->hot_swap_done = true;
    }

    // Handle resize
    if(context && context->resolution_changed)
    {
      init_size_dependent_resources(true);
    }

    // Handle command list
    graphics_pipeline->bind_command_list(command_list->com.Get());
    command_list->com.Get()->SetDescriptorHeaps(1, context->main_descriptor_heap->com.GetAddressOf());
    command_list->set_viewport(context->width, context->height);
    command_list->set_scissor(context->width, context->height);
  }

  bool fpass_base::can_draw() const
  {
    return pixel_shader_asset.is_loaded() && pixel_shader_asset.get()->compilation_successful
      && vertex_shader_asset.is_loaded() && vertex_shader_asset.get()->compilation_successful;
  }

  void fpass_base::update_vertex_and_index_buffers(fgraphics_command_list* command_list) const
  {
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    const uint32_t N = fmath::to_uint32(scene_acceleration.h_meshes.size());

    // Update vertex and index buffers
    for(uint32_t i = 0; i < N; i++)
    {
      hstatic_mesh* hmesh = scene_acceleration.h_meshes[i];
      astatic_mesh* amesh = hmesh->mesh_asset_ptr.get();
      if(!amesh->is_resource_online)
      {
        std::string mesh_name = hmesh->get_display_name();
        std::string asset_name = hmesh->mesh_asset_ptr.get()->name;
        command_list->upload_vertex_buffer(amesh, std::format("{} {}", mesh_name, asset_name).c_str());
        command_list->upload_index_buffer(amesh, std::format("{} {}", mesh_name, asset_name).c_str());
      }
    }
  }
  
  void fpass_base::upload_all_textures_once(fgraphics_command_list* command_list) const
  {
    static bool textures_uploaded = false;
    if(textures_uploaded) return;
    
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fdevice* device = fapplication::get_instance()->device.get();

    // Process all textures: create SRVs and upload
    // This should happen once in the first draw
    for(uint32_t i = 0; i < scene_acceleration.a_textures.size(); i++)
    {
      atexture* texture = scene_acceleration.a_textures[i];
      if(!texture->gpu_resource.com)
      {
        device->create_texture_buffer(heap, texture, texture->get_display_name().c_str());
      }
      if(!texture->is_online)
      {
        command_list->upload_texture(texture);
      }
    }
    textures_uploaded = true;
  }

  CD3DX12_GPU_DESCRIPTOR_HANDLE fpass_base::get_textures_gpu_handle() const
  {
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;

    return scene_acceleration.a_textures[0]->gpu_resource.srv.gpu_descriptor_handle;
  }
}