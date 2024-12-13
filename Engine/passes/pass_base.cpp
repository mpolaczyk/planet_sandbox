
#include "passes/pass_base.h"

#include "assets/texture.h"
#include "core/application.h"
#include "core/window.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "math/vertex_data.h"
#include "renderer/command_list.h"
#include "renderer/device.h"
#include "renderer/render_context.h"

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
    if(pixel_shader_asset.get()->hot_swap_requested || vertex_shader_asset.get()->hot_swap_requested)
    {
      LOG_INFO("Recreating pipeline state.")
      graphics_pipeline.reset(nullptr);
      init_pipeline();
      pixel_shader_asset.get()->hot_swap_done = true;
      vertex_shader_asset.get()->hot_swap_done = true;
    }

    context = in_context;
    if(context && context->resolution_changed)
    {
      init_size_dependent_resources(true);
    }
    command_list->set_viewport(context->width, context->height);
    command_list->set_scissor(context->width, context->height);
  }

  bool fpass_base::can_draw() const
  {
    return pixel_shader_asset.is_loaded() && pixel_shader_asset.get()->compilation_successful
      && vertex_shader_asset.is_loaded() && vertex_shader_asset.get()->compilation_successful;
  }

  void fpass_base::upload_all_textures(fgraphics_command_list* command_list)
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
}