
#include "engine/log.h"
#include "engine/hash.h"
#include "hittables/scene.h"
#include "object/object_registry.h"
#include "profile/benchmark.h"
#include "renderer/renderer_base.h"
#include "renderer/dx12_lib.h"

#include "core/application.h"

namespace engine
{
  OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
  OBJECT_DEFINE_NOSPAWN(rrenderer_base)

  rrenderer_base::rrenderer_base()
  {
    //scene.scene_acceleration = new fscene_acceleration();
  }

  rrenderer_base::~rrenderer_base()
  {
    //delete context.scene_acceleration;
  }

  void rrenderer_base::set_renderer_context(frenderer_context&& in_context)
  {
    if(!in_context.validate())
    {
      throw std::runtime_error("Failed to validate renderer context");
    }
    context = std::move(in_context);
  }
  
  void rrenderer_base::draw(ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    if(!can_draw())
    {
      return;
    }

    const uint32_t resolution_hash = fhash::combine(output_height, output_width);
    if(resolution_hash != last_frame_resolution_hash)
    {
      LOG_INFO("Recreating output texture");
      create_output_texture(true);
      last_frame_resolution_hash = resolution_hash;
    }

    // Initialize
    if(!init_done)
    {
      init();
      init_done = true;
    }

    if(!can_draw())
    {
      // Second check as something could've gone wrong in init()
      return;
    }
    
    context.scene->scene_acceleration.build(context.scene);
    if(!context.scene->scene_acceleration.validate())
    {
      return;
    }

    draw_internal(command_list);
    
  }

  bool rrenderer_base::can_draw()
  {
    if(output_height == 0 || output_width == 0)
    {
      LOG_ERROR("Can't draw. Incorrect resolution.");
      return false;
    }
    if(context.scene == nullptr)
    {
      LOG_ERROR("Can't draw. Scene is missing.");
      return false;
    }
    return true;
  }
}