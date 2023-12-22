
#include <ppl.h>
#include <sstream>
#include <cassert>

#include "hittables/hittables.h"
#include "math/camera.h"
#include "profile/benchmark.h"
#include "resources/bmp.h"
#include "object/object_registry.h"

#include "renderers/cpu_renderer_normals.h"

namespace engine
{
  OBJECT_DEFINE(cpu_renderer_normals, async_renderer_base, CPU renderer normals)
  OBJECT_DEFINE_SPAWN(cpu_renderer_normals)

  void cpu_renderer_normals::job_update()
  {
    std::vector<chunk> chunks;
    chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency() * 32, job_state.image_width, job_state.image_height, chunks);

    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](const chunk& ch) { render_chunk(ch); });
  }

  void cpu_renderer_normals::render_chunk(const chunk& in_chunk)
  {
    assert(job_state.scene_root != nullptr);
    std::thread::id thread_id = std::this_thread::get_id();

    std::ostringstream oss;
    oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
    engine::scope_counter benchmark_render_chunk(oss.str());

    for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
    {
      for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
      {
        // Effectively it is a fragment shader
        vec3 pixel_color;

        float u = float(x) / (job_state.image_width - 1);
        float v = float(y) / (job_state.image_height - 1);
        ray r = job_state.cam.get_ray(u, v);
        hit_record h;

        if (job_state.scene_root->hit(r, math::infinity, h))
        {
          pixel_color = (h.normal + 1.0f) * .5f;
        }

        bmp_pixel p(pixel_color);
        job_state.img_rgb->draw_pixel(x, y, &p, bmp_format::rgba);
        if (save_output)
        {
          job_state.img_bgr->draw_pixel(x, y, &p);
        }
      }
    }
  }
}