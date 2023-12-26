#pragma once

#include <string>
#include <thread>
#include <semaphore>

#include "resources/bmp.h"
#include "engine/hash.h"

#include "math/camera.h"
#include "hittables/hittables.h"
#include "hittables/scene.h"
#include "object/object.h"
#include "profile/benchmark.h"

namespace engine
{
  class ENGINE_API renderer_config
  {
  public:
    // Anti Aliasing oversampling
    int rays_per_pixel = 20;

    // Diffuse reflection
    int ray_bounces = 7;

    // How work is processed
    const class_object* type = nullptr;
    const class_object* new_type = nullptr;
    
    // Draw in the same memory - real time update
    bool reuse_buffer = true;

    int resolution_vertical = 0;
    int resolution_horizontal = 0;

    // Manually set the brightest point of an image used for tone mapping
    float white_point = 1.0f;

    inline uint32_t get_hash() const
    {
      return hash::combine(rays_per_pixel, ray_bounces, reuse_buffer, hash::combine(resolution_vertical, resolution_horizontal, hash::get(white_point), 1));
    }
  };

  // The responsibility of this class is to render to a texture
  class ENGINE_API async_renderer_base : public object
  {
  public:
    OBJECT_DECLARE(async_renderer_base, object)

    async_renderer_base() = default;
    virtual ~async_renderer_base();
    async_renderer_base(const async_renderer_base&) = default; // FIX w don't want to copy renderer but all objects are copyable... make it optional
    async_renderer_base& operator=(const async_renderer_base&) = default;
    async_renderer_base(async_renderer_base&&) = delete;
    async_renderer_base& operator=(async_renderer_base&&) = delete;
    
    // Worker thread public interface, implement rendering logic here
    virtual void job_init() {}        // Fired once in a lifetime
    virtual void job_pre_update();
    virtual void job_update() = 0;
    virtual void job_post_update();
    virtual void job_cleanup() {}     // Fired once in a lifetime
    
    // Main thread public interface.
    void render_frame(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config);
    void cancel();
    virtual bool is_async() const { return true; }
    bool is_working() const { return job_state.is_working; }
    uint64_t get_render_time() const { return job_state.benchmark_render_time; }
    uint64_t get_save_time() const { return job_state.benchmark_save_time; }
    uint64_t get_ray_count() const { return job_state.ray_count; }
    uint64_t get_ray_triangle_intersection_count() const { return job_state.ray_triangle_intersection_count; }
    uint64_t get_ray_box_intersection_count() const { return job_state.ray_box_intersection_count; }
    uint64_t get_ray_object_intersection_count() const { return job_state.ray_object_intersection_count; }
    uint8_t* get_img_bgr() const { return job_state.img_bgr->get_buffer(); }
    uint8_t* get_img_rgb() const { return job_state.img_rgb->get_buffer(); }

  protected:
    // Data assess pattern for async job state
    // - RW for worker thread
    // - R for main thread while processing, only through const getters
    struct worker_job_state
    {
      bool is_working = false;
      bool requested_stop = false;

      int image_height = 0;
      int image_width = 0;

      renderer_config renderer_conf;
      camera cam;
      const scene* scene_root = nullptr;
      uint32_t scene_root_hash = 0;

      bmp_image* img_bgr = nullptr;
      bmp_image* img_rgb = nullptr;

      uint64_t benchmark_render_time = 0;
      uint64_t benchmark_save_time = 0;
      uint64_t ray_count = 0;
      uint64_t ray_triangle_intersection_count = 0;
      uint64_t ray_box_intersection_count = 0;
      uint64_t ray_object_intersection_count = 0;
    };
    worker_job_state job_state;
    bool save_output = false; // FIX Expose to a UI setting
  
  private:
    void set_job_state(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config);
    // Synchronisation pattern through a semaphore
    std::thread* worker_thread = nullptr;
    std::binary_semaphore* worker_semaphore = nullptr;
    timer_instance benchmark_render;
    bool job_init_done = false;
    bool job_cleanup_done = false;
    void worker_function();
    void save(const char* file_name) const;
  };
}
