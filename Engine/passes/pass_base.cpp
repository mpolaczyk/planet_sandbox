
#include "passes/pass_base.h"

#include "core/application.h"
#include "core/window.h"
#include "engine/log.h"
#include "math/vertex_data.h"
#include "renderer/command_list.h"
#include "renderer/render_context.h"

namespace engine
{
  void fpass_base::init()
  {
    init_pipeline();
    init_size_independent_resources();
    init_size_dependent_resources(false);
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

  void fpass_base::draw(fgraphics_command_list* command_list)
  {
    if(pixel_shader_asset.get()->hot_swap_requested || vertex_shader_asset.get()->hot_swap_requested)
    {
      LOG_INFO("Recreating pipeline state.")
      graphics_pipeline.reset(nullptr);
      init_pipeline();
      pixel_shader_asset.get()->hot_swap_done = true;
      vertex_shader_asset.get()->hot_swap_done = true;
    }
    
    if(context && context->resolution_changed)
    {
      init_size_dependent_resources(true);
    }
    command_list->set_viewport(context->width, context->height);
    command_list->set_scissor(context->width, context->height);
  }

  bool fpass_base::get_can_draw() const
  {
    return pixel_shader_asset.get() != nullptr && vertex_shader_asset.get() != nullptr
      && pixel_shader_asset.get()->resource.blob && vertex_shader_asset.get()->resource.blob;
  }
}