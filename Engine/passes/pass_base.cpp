
#include "passes/pass_base.h"

#include "core/application.h"
#include "core/window.h"
#include "engine/log.h"
#include "renderer/aligned_structs.h"
#include "renderer/render_context.h"

namespace engine
{
  void fpass_base::init()
  {
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    if(!vertex_shader_asset.get()->render_state.blob)
    {
      LOG_ERROR("Failed to load vertex shader.");
      can_draw = false;
      return;
    }
    if(!pixel_shader_asset.get()->render_state.blob)
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
  }
}