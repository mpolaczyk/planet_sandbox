#include <ppl.h>
#include <sstream>
#include <assert.h>

#include "hittables/hittables.h"
#include "math/camera.h"
#include "profile/benchmark.h"

#include "renderers/cpu_renderer_faces.h"

#include "math/colors.h"
#include "resources/bmp.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(cpu_renderer_faces, cpu_renderer_base, CPU renderer faces)
  OBJECT_DEFINE_SPAWN(cpu_renderer_faces)

  std::string cpu_renderer_faces::get_name() const
  {
    return "CPU Preview Normals";
  }

  void cpu_renderer_faces::render()
  {
    save_output = false;

    std::vector<chunk> chunks;
    chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency() * 32, state.image_width, state.image_height, chunks);

    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](const chunk& ch) { render_chunk(ch); });
  }

  void cpu_renderer_faces::render_chunk(const chunk& in_chunk)
  {
    assert(state.scene_root != nullptr);
    assert(state.cam != nullptr);
    std::thread::id thread_id = std::this_thread::get_id();

    std::ostringstream oss;
    oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
    engine::scope_counter benchmark_render_chunk(oss.str(), false);

    for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
    {
      for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
      {
        // Effectively it is a fragment shader
        vec3 pixel_color;

        float u = float(x) / (state.image_width - 1);
        float v = float(y) / (state.image_height - 1);
        ray r = state.cam->get_ray(u, v);
        hit_record h;

        if (state.scene_root->hit(r, 0.01f, math::infinity, h))
        {
          int color_index = (static_cast<hittable*>(h.object)->get_runtime_id() + h.face_id) % colors::num;
          pixel_color = colors::all[color_index];
        }

        bmp_pixel p(pixel_color);
        state.img_rgb->draw_pixel(x, y, &p, bmp_format::rgba);
        if (save_output)
        {
          state.img_bgr->draw_pixel(x, y, &p);
        }
      }
    }
  }
}