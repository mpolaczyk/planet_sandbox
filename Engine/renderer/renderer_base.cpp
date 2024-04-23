
#include <d3d11_1.h>

#include "renderer/dx11_lib.h"

#include "renderer/renderer_base.h"

#include "engine/log.h"
#include "object/object_registry.h"

namespace engine
{
    OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
    OBJECT_DEFINE_NOSPAWN(rrenderer_base) 

    void rrenderer_base::init()
    {

    }

  void rrenderer_base::start_frame_timer()
  {
      timestamp_start = 0;
      perf_counter_frequency = 0;
      {
        LARGE_INTEGER perf_count;
        QueryPerformanceCounter(&perf_count);
        timestamp_start = perf_count.QuadPart;
        LARGE_INTEGER perf_freq;
        QueryPerformanceFrequency(&perf_freq);
        perf_counter_frequency = perf_freq.QuadPart;
      }
      benchmark_renderer.start("Render");
  }

  void rrenderer_base::stop_frame_timer()
  {
      LARGE_INTEGER perf_count;
      QueryPerformanceCounter(&perf_count);
      const LONGLONG timestamp_now = perf_count.QuadPart;
      delta_time = static_cast<double>(timestamp_now - timestamp_start) / static_cast<double>(perf_counter_frequency);
      benchmark_renderer.stop();
  }

  void rrenderer_base::render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera_config& in_camera_config)
  {
    camera = in_camera_config;
    scene = in_scene;

    const bool size_changed = output_width != in_renderer_config.resolution_horizontal
                          || output_height != in_renderer_config.resolution_vertical;
    output_width = in_renderer_config.resolution_horizontal;
    output_height = in_renderer_config.resolution_vertical;
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
  }
}