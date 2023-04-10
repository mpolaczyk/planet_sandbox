#include "stdafx.h"

#include <ppl.h>

#include "example_renderer.h"

std::string example_renderer::get_name() const
{
  return "Example";
}

void example_renderer::render()
{
  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency(), ajs.image_width, ajs.image_height, chunks);

  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(ch); });
}

void example_renderer::render_chunk(const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  const char* name = oss.str().c_str();
  benchmark::scope_counter benchmark_render_chunk(name, false);

  for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      // Effectively it is a fragment shader
      vec3 pixel_color;

      int option = 3;
      if (option == 1)
      {
        float u = float(x) / (ajs.image_width - 1);
        float v = float(y) / (ajs.image_height - 1);
        pixel_color = vec3(u, v, 0.3f);
      }
      else if (option == 2)
      {
        pixel_color = vec3(abs(sin(x / 50.0f)), abs(sin(y / 300.0f)), 0.5f);
      }
      else if (option == 3)
      {
        if (x < ajs.image_width / 2)
        {
          // Cached noise
          pixel_color = rand_direction();
        }
        else
        {
          // Runtime noise
          pixel_color = rand_direction(ajs.image_width * ajs.image_height);
        }
      }

      bmp::bmp_pixel p(pixel_color);
      ajs.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
      if (save_output)
      {
        ajs.img_bgr->draw_pixel(x, y, &p);
      }
    }
  }
}