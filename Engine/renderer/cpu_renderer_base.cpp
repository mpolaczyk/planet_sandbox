#include <cassert>
#include <thread>
#include <semaphore>

#include "cpu_renderer_base.h"

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
  OBJECT_DEFINE(cpu_renderer_base, object, CPU renderer base)
  OBJECT_DEFINE_NOSPAWN(cpu_renderer_base)

  cpu_renderer_base::cpu_renderer_base()
  {
    worker_thread = new std::thread(&cpu_renderer_base::async_job, this);
    worker_semaphore = new std::binary_semaphore(0);
  }
  cpu_renderer_base::~cpu_renderer_base()
  {
    state.requested_stop = true;
    worker_semaphore->release();
    worker_thread->join();
    delete worker_semaphore;
    delete worker_thread;
    delete state.scene_root;
    if (state.img_rgb != nullptr) delete state.img_rgb;
    if (state.img_bgr != nullptr) delete state.img_bgr;
  }

  void cpu_renderer_base::set_config(const renderer_config& in_renderer_config, scene* in_scene, const camera_config& in_camera_config)
  {
    assert(in_scene != nullptr);

    if (state.is_working) return;

    bool force_recreate_buffers = state.image_width != in_renderer_config.resolution_horizontal || state.image_height != in_renderer_config.resolution_vertical;

    // Copy all objects on purpose
    // - allows original scene to be edited while this one is rendering
    // - allows to detect if existing is dirty
    state.image_width = in_renderer_config.resolution_horizontal;
    state.image_height = in_renderer_config.resolution_vertical;
    state.renderer_conf = in_renderer_config;
    state.scene_root = in_scene;
    state.scene_root_hash = in_scene->get_hash();
    state.cam.configure(in_camera_config);

    // Delete buffers 
    if (state.img_rgb != nullptr)
    {
      if (force_recreate_buffers || !state.renderer_conf.reuse_buffer)
      {
        delete state.img_rgb;
        delete state.img_bgr;
        state.img_rgb = nullptr;
        state.img_bgr = nullptr;
      }
    }

    // Create new buffers if they are missing
    if (state.img_rgb == nullptr)
    {
      state.img_rgb = new bmp_image(state.image_width, state.image_height);
      state.img_bgr = new bmp_image(state.image_width, state.image_height);
    }
  }

  void cpu_renderer_base::render_single_async()
  {
    if (state.is_working) return;

    state.is_working = true;
    worker_semaphore->release();
  }

  bool cpu_renderer_base::is_world_dirty(const scene* in_scene) const
  {
    assert(in_scene != nullptr);
    return state.scene_root_hash != in_scene->get_hash();
  }

  bool cpu_renderer_base::is_renderer_setting_dirty(const renderer_config& in_renderer_config) const
  {
    return state.renderer_conf.get_hash() != in_renderer_config.get_hash();
  }

  bool cpu_renderer_base::is_renderer_type_different(const renderer_config& in_renderer_config) const
  {
    return state.renderer_conf.type != in_renderer_config.type;
  }

  bool cpu_renderer_base::is_camera_setting_dirty(const camera_config& in_camera_config) const
  {
    return state.cam.get_hash() != in_camera_config.get_hash();
  }

  void cpu_renderer_base::async_job()
  {
    while (true)
    {
      worker_semaphore->acquire();
      if (state.requested_stop) { break; }

      stats::reset();
      instance benchmark_render;
      benchmark_render.start("Render");

      render();

      state.ray_count = stats::get_ray_count();
      state.ray_triangle_intersection_count = stats::get_ray_triangle_intersection_count();
      state.ray_box_intersection_count = stats::get_ray_box_intersection_count();
      state.ray_object_intersection_count = stats::get_ray_object_intersection_count();
      state.benchmark_render_time = benchmark_render.stop();

      if (save_output)
      {
        char image_file_name[300];  // Run-Time Check Failure #2 - Stack around the variable 'image_file_name' was corrupted.
        std::sprintf(image_file_name, io::get_render_output_file_path().c_str());

        instance benchmark_save;
        benchmark_save.start("Save");
        save(image_file_name);
        state.benchmark_save_time = benchmark_save.stop();
      }

      state.is_working = false;
    }
  }

  void cpu_renderer_base::save(const char* file_name)
  {
    state.img_bgr->save_to_file(file_name);
  }
}