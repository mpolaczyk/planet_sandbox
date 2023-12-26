#pragma once

#include <thread>
#include <semaphore>

#include "renderer/renderer_base.h"

namespace engine
{
  // CPU renderer renders to the texture (rgb buffer)
  // It is using a background worker thread so that a caller can continue.
  // Can take multiple seconds/minutes to finish.
  class ENGINE_API cpu_renderer : public renderer_base
  {
  public: 
    OBJECT_DECLARE(cpu_renderer, renderer_base)
 
    // Main thread public interface.
    virtual void render_frame(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config) override;
    virtual void push_partial_update() override;
    virtual void cancel() override { job_state.requested_stop = true; }
    virtual bool is_async() const override { return true; }
    virtual bool is_working() const override { return job_state.is_working; }
    virtual bool is_cancelled() const override { return job_state.requested_stop; }
    virtual void cleanup() override;
    
    uint64_t get_render_time() const { return job_state.benchmark_render_time; }
    uint64_t get_save_time() const { return job_state.benchmark_save_time; }
    uint64_t get_ray_count() const { return job_state.ray_count; }
    uint64_t get_ray_triangle_intersection_count() const { return job_state.ray_triangle_intersection_count; }
    uint64_t get_ray_box_intersection_count() const { return job_state.ray_box_intersection_count; }
    uint64_t get_ray_object_intersection_count() const { return job_state.ray_object_intersection_count; }
    uint8_t* get_img_bgr() const { return job_state.img_bgr->get_buffer(); }
    uint8_t* get_img_rgb() const { return job_state.img_rgb->get_buffer(); }

  protected:
    // A copy of the world at the time of a call
    struct worker_job_state
    {
      bool is_working = false;
      bool requested_stop = false;
      bool can_partial_update = false;
      
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
    // Data assess pattern for async job state
    // - RW for worker thread
    // - R for main thread while processing, only through const getters
    worker_job_state job_state;
    void set_job_state(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config);

    // Synchronisation pattern through a semaphore
    std::thread* worker_thread = nullptr;
    std::binary_semaphore* worker_semaphore = nullptr;
    void worker_function();
    // Worker thread protected interface, implement rendering logic here
    virtual void job_pre_update();
    virtual void job_update() = 0;
    virtual void job_post_update();
    timer_instance benchmark_render;
    
    bool save_output = false; // FIX Expose to a UI setting
    void save(const char* file_name) const;    
  };
}