
#include <d3d11_1.h>

#include "renderer/dx11_lib.h"
#include "renderer/renderer_base.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "object/object_registry.h"
#include "profile/benchmark.h"

namespace engine
{
  OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
  OBJECT_DEFINE_NOSPAWN(rrenderer_base) 

  void rrenderer_base::render_frame(const hscene* in_scene)
  {
    if(in_scene == nullptr)
    {
        LOG_ERROR("Can't render. Bad scene.");
        return;
    }
    const frenderer_config& renderer_config = in_scene->renderer_config;
      
    if (renderer_config.resolution_vertical == 0 || renderer_config.resolution_horizontal == 0) return;
    camera = in_scene->camera_config;
    scene = in_scene;

    const bool size_changed = output_width != renderer_config.resolution_horizontal
                          || output_height != renderer_config.resolution_vertical;
    output_width = renderer_config.resolution_horizontal;
    output_height = renderer_config.resolution_vertical;
    if (size_changed)
    {
      LOG_INFO("Recreating output texture");
      create_output_texture(true);
    }
    
    // Initialize
    if (!init_done)
    {
      init();
      init_done = true;
    }

    uint64_t render_time_us;
    {
        fscope_timer benchmark_renderer("Render", &render_time_us);
        render_frame_impl();
    }
    render_time_ms = static_cast<double>(render_time_us) / 1000;
  }

  void rrenderer_base::create_output_texture(bool cleanup)
    {
        if (cleanup)
        {
            DX_RELEASE(output_rtv)
            DX_RELEASE(output_srv)
            DX_RELEASE(output_dsv)
            DX_RELEASE(output_texture)
        }
        
        fdx11& dx = fdx11::instance();
        dx.create_render_target_shader_resource_view(output_width, output_height, output_texture, output_srv);
        dx.create_render_target_view(output_texture, output_rtv);
        dx.create_depth_stencil_view(output_width, output_height, output_dsv);
    }
}
