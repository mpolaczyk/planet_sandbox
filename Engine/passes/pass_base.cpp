
#include "passes/pass_base.h"

#include "core/application.h"
#include "core/window.h"
#include "engine/log.h"
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
    init_size_dependent(false);
  }

  void fpass_base::draw(fgraphics_command_list* command_list)
  {
    if(context && context->resolution_changed)
    {
      init_size_dependent(true);
    }
    command_list->set_viewport(context->width, context->height);
    command_list->set_scissor(context->width, context->height);
  }
}