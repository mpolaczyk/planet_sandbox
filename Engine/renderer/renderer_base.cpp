
#include "engine/log.h"
#include "engine/hash.h"
#include "hittables/scene.h"
#include "object/object_registry.h"
#include "profile/benchmark.h"
#include "renderer/renderer_base.h"
#include "renderer/aligned_structs.h"

namespace engine
{
  OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
  OBJECT_DEFINE_NOSPAWN(rrenderer_base)

  void rrenderer_base::render_frame(ComPtr<ID3D12GraphicsCommandList> command_list, fwindow* in_window, const hscene* in_scene, const hhittable_base* in_selected_object)
  {
    scene = in_scene;
    selected_object = in_selected_object;
    window = in_window;

    if(!can_render())
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

    uint64_t render_time_us;
    {
      fscope_timer benchmark_renderer("Render", &render_time_us);
      
      scene_acceleration.clean(MAX_LIGHTS, MAX_MATERIALS);
      scene_acceleration.build(scene->objects);
      if(!scene_acceleration.validate())
      {
        return;
      }
      
      render_frame_internal(command_list);
    }
    render_time_ms = static_cast<double>(render_time_us) / 1000;
  }

  bool rrenderer_base::can_render()
  {
    if(output_height == 0 || output_width == 0)
    {
      LOG_ERROR("Can't render. Incorrect resolution.");
      return false;
    }
    if(scene == nullptr)
    {
      LOG_ERROR("Can't render. Scene is missing.");
      return false;
    }
    return true;
  }
}