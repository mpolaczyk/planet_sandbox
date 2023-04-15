#pragma once

#include <thread>
#include <semaphore>

#include "math/vec3.h"
#include "math/hittables.h"
#include "math/ray.h"
#include "chunk_generator.h"
#include "math/camera.h"
#include "gfx/bmp.h"

#include "app/json/serializable.h"

namespace bmp
{
  struct bmp_image;
}
class camera;

enum class threading_strategy_type
{
  none = 0,
  pll_for_each,
  thread_pool
};
static inline const char* threading_strategy_names[] =
{
  "None",
  "PLL for each",
  "Thread poll"
};

class renderer_config : serializable<nlohmann::json>
{
public:
  // Anti Aliasing oversampling
  int rays_per_pixel = 20;

  // Diffuse reflection
  int ray_bounces = 7;

  // How work is processed
  threading_strategy_type threading_strategy = threading_strategy_type::thread_pool;
    
  // Draw in the same memory - real time update
  bool reuse_buffer = true;

  int resolution_vertical = 0;
  int resolution_horizontal = 0;

  nlohmann::json serialize();
  void deserialize(const nlohmann::json& j);

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(renderer_config, rays_per_pixel, ray_bounces, threading_strategy, reuse_buffer, resolution_vertical, resolution_horizontal); // to_json only

  inline uint32_t get_type_hash() const
  {
    return hash_combine(rays_per_pixel, ray_bounces);
  }
};

class async_renderer_base
{
public:
  async_renderer_base();
  ~async_renderer_base();

  // Renderer instance interface
  virtual std::string get_name() const = 0;
  virtual void render() = 0;

  // Renderer public interface. Usage:
  // 1. Set scene, camera and settings first
  void set_config(const renderer_config& in_settings, const scene& in_scene, const camera_config& in_camera_state);
  // 2. Request work
  void render_single_async();
  
  // State checks
  bool is_world_dirty(const scene& in_scene);
  bool is_renderer_setting_dirty(const renderer_config& in_settings);
  bool is_camera_setting_dirty(const camera_config& in_camera_state);
  bool is_working() const { return ajs.is_working; }

  uint64_t get_render_time() const { return ajs.benchmark_render_time; }
  uint64_t get_save_time() const { return ajs.benchmark_save_time; }
  uint8_t* get_img_bgr() { return ajs.img_bgr->get_buffer(); }
  uint8_t* get_img_rgb() { return ajs.img_rgb->get_buffer(); }

  bool save_output = true;

protected:
  // Data assess pattern for async job state
  // - RW for job thread
  // - R for main thread while processing, only through const getters
  // - W for game thread only using set_config
  struct 
  {
    bool is_working = false;
    bool requested_stop = false;

    uint32_t image_height = 0;
    uint32_t image_width = 0;

    renderer_config settings;
    camera cam;
    scene scene_root;
    
    bmp::bmp_image* img_bgr = nullptr;
    bmp::bmp_image* img_rgb = nullptr;

    uint64_t benchmark_render_time = 0;
    uint64_t benchmark_save_time = 0;
  } ajs; 

private:
  // Synchronization - fire and forget
  // No job cancellation
  std::thread worker_thread;
  std::binary_semaphore worker_semaphore{ 0 };
  void async_job();
  void save(const char* file_name);
};