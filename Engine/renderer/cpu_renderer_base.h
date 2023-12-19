#pragma once

#include <string>
#include <thread>
#include <semaphore>


#include "math/vec3.h"
#include "resources/bmp.h"
#include "engine/hash.h"

#include "math/camera.h"
#include "hittables/hittables.h"
#include "hittables/scene.h"
#include "object/object.h"

//// Forward declaration for std::binary_semaphore    FIX
//#pragma warning(disable:4091)
//// warning C4091: 'typedef ': ignored on left of '__int64' when no variable is declared
//namespace std
//{
//  class thread;
//  typedef ptrdiff_t;
//  template <ptrdiff_t _Least_max_value>
//  class counting_semaphore;
//  using binary_semaphore = counting_semaphore<1>;
//}
//#pragma warning(default:4091)

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

  // Data assess pattern for async job state
  // - RW for job thread
  // - R for main thread while processing, only through const getters
  // - W for game thread only using set_config
  struct ENGINE_API job_state
  {
    bool is_working = false;
    bool requested_stop = false;

    int image_height = 0;
    int image_width = 0;

    renderer_config renderer_conf;
    camera* cam = nullptr;
    scene* scene_root = nullptr;
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

  class ENGINE_API cpu_renderer_base : public object
  {
  public:
    OBJECT_DECLARE(cpu_renderer_base, object)

    cpu_renderer_base();
    ~cpu_renderer_base();

    // Renderer instance interface
    virtual void render() = 0;

    // Renderer public interface. Usage:
    // 1. Set scene, camera and settings first
    void set_config(const renderer_config& in_renderer_config, scene* in_scene, const camera_config& in_camera_config);
    // 2. Request work
    void render_single_async();

    // State checks
    bool is_world_dirty(const scene* in_scene);
    bool is_renderer_setting_dirty(const renderer_config& in_renderer_config);
    bool is_renderer_type_different(const renderer_config& in_renderer_config);
    bool is_camera_setting_dirty(const camera_config& in_camera_config);
    bool is_working() const { return state.is_working; }

    uint64_t get_render_time() const { return state.benchmark_render_time; }
    uint64_t get_save_time() const { return state.benchmark_save_time; }
    uint64_t get_ray_count() const { return state.ray_count; }
    uint64_t get_ray_triangle_intersection_count() const { return state.ray_triangle_intersection_count; }
    uint64_t get_ray_box_intersection_count() const { return state.ray_box_intersection_count; }
    uint64_t get_ray_object_intersection_count() const { return state.ray_object_intersection_count; }
    uint8_t* get_img_bgr() { return state.img_bgr->get_buffer(); }
    uint8_t* get_img_rgb() { return state.img_rgb->get_buffer(); }

    bool save_output = true;

  protected:
    job_state state;

  private:
    // Synchronization - fire and forget
    // No job cancellation
    std::thread* worker_thread = nullptr;
    std::binary_semaphore* worker_semaphore = nullptr;
    void async_job();
    void save(const char* file_name);
  };
}