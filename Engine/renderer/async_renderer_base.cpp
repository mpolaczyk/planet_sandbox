#include <cassert>
#include <thread>
#include <semaphore>

#include "async_renderer_base.h"

#include "resources/bmp.h"

#include "math/pdf.h"
#include "math/camera.h"
#include "profile/benchmark.h"
#include "profile/stats.h"
#include "engine/io.h"
#include "hittables/hittables.h"
#include "object/object_registry.h"
#include "hittables/scene.h"

namespace engine
{
  OBJECT_DEFINE(async_renderer_base, object, Async renderer base)
  OBJECT_DEFINE_NOSPAWN(async_renderer_base)

  async_renderer_base::async_renderer_base()
  {
    worker_thread = new std::thread(&async_renderer_base::worker_function, this);
    worker_semaphore = new std::binary_semaphore(0);
  }
  async_renderer_base::~async_renderer_base()
  {
    job_state.requested_stop = true;
    worker_semaphore->release();
    worker_thread->join();
    delete worker_semaphore;
    delete worker_thread;
    delete job_state.img_rgb;
    delete job_state.img_bgr;
  }

  void async_renderer_base::set_job_state(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config)
  {
    assert(in_scene != nullptr);

    if (job_state.is_working) return;

    const bool force_recreate_buffers = job_state.image_width != in_renderer_config.resolution_horizontal || job_state.image_height != in_renderer_config.resolution_vertical;

    // Copy all objects on purpose
    // - allows original scene to be edited while this one is rendering
    // - allows to detect if existing is dirty
    job_state.image_width = in_renderer_config.resolution_horizontal;
    job_state.image_height = in_renderer_config.resolution_vertical;
    job_state.renderer_conf = in_renderer_config;
    job_state.scene_root = in_scene;
    job_state.scene_root_hash = in_scene->get_hash();
    job_state.cam.configure(in_camera_config);

    // Delete buffers 
    if (job_state.img_rgb != nullptr)
    {
      if (force_recreate_buffers || !job_state.renderer_conf.reuse_buffer)
      {
        delete job_state.img_rgb;
        delete job_state.img_bgr;
        job_state.img_rgb = nullptr;
        job_state.img_bgr = nullptr;
      }
    }

    // Create new buffers if they are missing
    if (job_state.img_rgb == nullptr)
    {
      job_state.img_rgb = new bmp_image(job_state.image_width, job_state.image_height);
      job_state.img_bgr = new bmp_image(job_state.image_width, job_state.image_height);
    }
  }

  void async_renderer_base::render_single_async(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config)
  {
    if (job_state.is_working) return;
    
    set_job_state(in_scene, in_renderer_config, in_camera_config);

    job_state.is_working = true;
    worker_semaphore->release();
  }

  bool async_renderer_base::is_world_dirty(const scene* in_scene) const
  {
    assert(in_scene != nullptr);
    return job_state.scene_root_hash != in_scene->get_hash();
  }

  bool async_renderer_base::is_renderer_setting_dirty(const renderer_config& in_renderer_config) const
  {
    return job_state.renderer_conf.get_hash() != in_renderer_config.get_hash();
  }

  bool async_renderer_base::is_renderer_type_different(const renderer_config& in_renderer_config) const
  {
    return job_state.renderer_conf.type != in_renderer_config.type;
  }

  bool async_renderer_base::is_camera_setting_dirty(const camera_config& in_camera_config) const
  {
    return job_state.cam.get_hash() != in_camera_config.get_hash();
  }

  void async_renderer_base::worker_function()
  {
    job_init();
    while (true)
    {
      worker_semaphore->acquire();
      if (job_state.requested_stop) { break; }

      stats::reset();
      instance benchmark_render;
      benchmark_render.start("Render");

      job_update();

      job_state.ray_count = stats::get_ray_count();
      job_state.ray_triangle_intersection_count = stats::get_ray_triangle_intersection_count();
      job_state.ray_box_intersection_count = stats::get_ray_box_intersection_count();
      job_state.ray_object_intersection_count = stats::get_ray_object_intersection_count();
      job_state.benchmark_render_time = benchmark_render.stop();

      if (save_output)
      {
        char image_file_name[300];  // Run-Time Check Failure #2 - Stack around the variable 'image_file_name' was corrupted.
        std::sprintf(image_file_name, io::get_render_output_file_path().c_str());

        instance benchmark_save;
        benchmark_save.start("Save");
        save(image_file_name);
        job_state.benchmark_save_time = benchmark_save.stop();
      }

      job_state.is_working = false;
    }
    job_cleanup();
  }

  void async_renderer_base::save(const char* file_name) const
  {
    job_state.img_bgr->save_to_file(file_name);
  }
}