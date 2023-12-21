
#include <ppl.h>
#include <sstream>
#include <cassert>

#include "math/camera.h"
#include "profile/benchmark.h"

#include "renderers/cpu_renderer_reference.h"

#include "math/colors.h"
#include "math/random.h"
#include "math/math.h"
#include "profile/stats.h"
#include "resources/bmp.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(cpu_renderer_reference, cpu_renderer_base, CPU renderer reference)
  OBJECT_DEFINE_SPAWN(cpu_renderer_reference)

  void cpu_renderer_reference::render()
  {
    std::vector<chunk> chunks;
    const int chunks_per_thread = 32;
    chunk_generator::generate_chunks(chunk_strategy_type::rectangles, std::thread::hardware_concurrency() * chunks_per_thread, state.image_width, state.image_height, chunks);

    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](const chunk& ch) { render_chunk(ch); });
  }


  void cpu_renderer_reference::render_chunk(const chunk& in_chunk)
  {
    std::thread::id thread_id = std::this_thread::get_id();

    std::ostringstream oss;
    oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
    engine::scope_counter benchmark_render_chunk(oss.str(), false);

    vec3 resolution((float)state.image_width, (float)state.image_height, 0.0f);

    for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
    {
      for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
      {
        vec3 hdr_color = fragment((float)x, (float)y, resolution);

        assert(isfinite(hdr_color.x));
        assert(isfinite(hdr_color.y));
        assert(isfinite(hdr_color.z));

        vec3 ldr_color = tone_mapping::reinhard_extended(hdr_color, state.renderer_conf.white_point);

        bmp_pixel p(ldr_color);
        state.img_rgb->draw_pixel(x, y, &p, bmp_format::rgba);
        if (save_output)
        {
          state.img_bgr->draw_pixel(x, y, &p);
        }
      }
    }
  }

  vec3 cpu_renderer_reference::fragment(float x, float y, const vec3& resolution)
  {
    uint32_t seed = uint32_t(y * resolution.x + x);  // Each pixel has a unique seed, gradient from white to black

    const int rays_per_pixel = state.renderer_conf.rays_per_pixel;
    vec3 sum_colors;

    for (int i = 0; i < rays_per_pixel; ++i)
    {
      // Anti aliasing with ray variance
      float u = (float(x) + random_seed::rand_pcg(seed)) / (resolution.x - 1);
      float v = (float(y) + random_seed::rand_pcg(seed)) / (resolution.y - 1);
      // Trace the ray
      ray r = state.cam.get_ray(u, v);
      sum_colors += trace_ray(r, seed);
    }

    vec3 hdr_color = sum_colors / (float)rays_per_pixel;

    return hdr_color;
  }

  vec3 cpu_renderer_reference::enviroment_light(const ray& in_ray)
  {
    static const vec3 sky_color_zenith = colors::white_blue;
    static const vec3 sky_color_horizon = colors::white;
    static const float sky_brightness = 0.4f;

    float t = math::smoothstep(-0.6f, 0.2f, in_ray.direction.y);
    vec3 light = math::lerp_vec3(sky_color_horizon, sky_color_zenith, t);
    return math::clamp_vec3(0.0f, 1.0f, light) * sky_brightness;
  }

  vec3 cpu_renderer_reference::trace_ray(ray in_ray, uint32_t seed)
  {
    assert(state.scene_root != nullptr);
    // Defined by material color of all bounces, mixing colors (multiply to aggregate) [0.0f, 1.0f]
    vec3 ray_color = vec3(1.0f);

    // Defined by emitted color of all emissive bounces weighted by the material color (add to aggregate) [0.0f, inf] 
    // Allow light to exceed 1.0f. Non-emissive materials can emit a little bit of light
    vec3 incoming_light = vec3(0.0f);

    int bounces = state.renderer_conf.ray_bounces;
    for (int i = 0; i < bounces; ++i)
    {
      stats::inc_ray();
      hit_record hit;
      if (state.scene_root->hit(in_ray, math::infinity, hit))
      {
        // Don't bounce if ray has no color
        if (math::length_squared(ray_color) < 0.1f)
        {
          break;
        }

        // Read material
        material_asset mat = *hit.material_ptr;

        if (mat.is_light)
        {
          // Don't bounce of lights
          incoming_light += mat.emitted_color * ray_color;
          break;
        }

        in_ray.origin = hit.p;

        bool can_gloss_bounce = mat.gloss_probability >= random_seed::rand_pcg(seed);
        bool can_refract = mat.refraction_probability >= random_seed::rand_pcg(seed);

        vec3 diffuse_dir = math::normalize(hit.normal + random_seed::direction(seed));

        if (!can_gloss_bounce && can_refract)
        {
          float refraction_ratio = hit.front_face ? (1.0f / mat.refraction_index) : mat.refraction_index;
          vec3 refraction_dir = math::refract(in_ray.direction, hit.normal, refraction_ratio);

          // Define next bounce
          in_ray.direction = refraction_dir + diffuse_dir * (1.0f - mat.smoothness);

          // Refraction color
          ray_color *= mat.color;

          assert(colors::is_valid(ray_color));
          continue;
        }

        // New directions

        vec3 specular_dir = math::reflect(in_ray.direction, hit.normal);

        // Define next bounce
        float blend = mat.smoothness;
        if (mat.gloss_probability > 0.0f)
        {
          blend = mat.smoothness * can_gloss_bounce;
        }
        in_ray.direction = math::lerp_vec3(diffuse_dir, specular_dir, blend);

        // Calculate color for this hit
        incoming_light += mat.emitted_color * ray_color;
        ray_color *= math::lerp_vec3(mat.color, mat.gloss_color, can_gloss_bounce);
        assert(colors::is_valid(ray_color));
      }
      else
      {
        vec3 env_light = enviroment_light(in_ray);  // environment is emissive
        incoming_light += env_light * ray_color;
        break;
      }
    }

    return incoming_light;
  }
}