
#include "passes/pass_base.h"

#include "core/application.h"
#include "core/window.h"
#include "engine/log.h"
#include "math/vertex_data.h"
#include "renderer/aligned_structs.h"
#include "renderer/command_list.h"
#include "renderer/render_context.h"

namespace engine
{
  void fpass_base::init()
  {
    if(!vertex_shader_asset.get()->resource.blob)
    {
      LOG_ERROR("Failed to load vertex shader.");
      can_draw = false;
      return;
    }
    if(!pixel_shader_asset.get()->resource.blob)
    {
      LOG_ERROR("Failed to load pixel shader.");
      can_draw = false;
      return;
    }
    if(MAX_TEXTURES + context->back_buffer_count * (MAX_MATERIALS + MAX_LIGHTS) > MAX_MAIN_DESCRIPTORS)
    {
      LOG_ERROR("Invalid main heap layout.");
      can_draw = false;
      return;
    }
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
    if(context && context->resolution_changed)
    {
      init_size_dependent_resources(true);
    }
    command_list->set_viewport(context->width, context->height);
    command_list->set_scissor(context->width, context->height);
  }
}