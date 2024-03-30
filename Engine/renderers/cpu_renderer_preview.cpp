#include <sstream>
#include <cassert>
#include <ppl.h>

#include "hittables/hittables.h"
#include "math/camera.h"
#include "profile/benchmark.h"

#include "renderers/cpu_renderer_preview.h"
#include "resources/bmp.h"
#include "engine/log.h"
#include "math/math.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(rcpu_preview, rcpu, CPU renderer preview)
  OBJECT_DEFINE_SPAWN(rcpu_preview)

  void rcpu_preview::job_update()
  {
    std::vector<fchunk> chunks;
    fchunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency() * 32, job_state.image_width, job_state.image_height, chunks);

    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](const fchunk& ch) { render_chunk(ch); });
  }

  void rcpu_preview::render_chunk(const fchunk& in_chunk)
  {
    assert(job_state.scene_root != nullptr);
    std::thread::id thread_id = std::this_thread::get_id();

    std::ostringstream oss;
    oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
    engine::fscope_timer benchmark_render_chunk(oss.str());

    hhittable_base* l = job_state.scene_root->lights[0];
    if (l == nullptr)
    {
      LOG_ERROR("Scene needs at least one light source.");
      return;
    }
    fvec3 light = l->get_origin();

    for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
    {
      for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
      {
        if(is_cancelled()) { return; }
        // Effectively it is a fragment shader
        fvec3 pixel_color;

        float u = float(x) / (job_state.image_width - 1);
        float v = float(y) / (job_state.image_height - 1);
        fray r = job_state.cam.get_ray(u, v);
        fhit_record h;

        if (job_state.scene_root->hit(r, fmath::infinity, h))
        {
          r.origin = h.p;
          fvec3 light_dir = fmath::normalize(light - r.origin);
          r.direction = light_dir;
          fhit_record sh;
          bool in_shadow = false;
          if (job_state.scene_root->hit(r, fmath::infinity, sh))
          {
            assert(sh.material_ptr != nullptr);
            in_shadow = !sh.material_ptr->is_light;
          }

          assert(h.material_ptr != nullptr);
          pixel_color = h.material_ptr->color * fmath::max1(0.2f, fmath::dot(h.normal, light_dir));
          if (in_shadow)
          {
            pixel_color *= fvec3(.9f, .9f, .9f);
          }
        }

        fbmp_pixel p(pixel_color);
        job_state.img_rgb->draw_pixel(x, y, &p, bmp_format::rgba);
        if (save_output)
        {
          job_state.img_bgr->draw_pixel(x, y, &p);
        }
      }
    }
  }
}