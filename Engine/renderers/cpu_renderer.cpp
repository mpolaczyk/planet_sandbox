
#include <thread>
#include <semaphore>

#include "renderers/cpu_renderer.h"

#include "renderer/dx11_lib.h"
#include "engine/io.h"
#include "object/object_registry.h"
#include "profile/stats.h"

namespace engine
{
  OBJECT_DEFINE(rcpu, rrenderer_base, CPU renderer)
  OBJECT_DEFINE_NOSPAWN(rcpu)
  
  void rcpu::destroy()
  {
    cancel();
    worker_semaphore->release();
    worker_thread->join();
    delete worker_semaphore;
    delete worker_thread;
    delete job_state.img_rgb;
    delete job_state.img_bgr;
    rrenderer_base::destroy();
  }
  
  void rcpu::set_job_state(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera_config& in_camera_config)
  {
    assert(in_scene != nullptr);

    if (job_state.is_working) return;
    if (in_renderer_config.resolution_vertical == 0|| in_renderer_config.resolution_horizontal == 0) return;
    
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
      job_state.img_rgb = new fbmp_image(job_state.image_width, job_state.image_height);
      job_state.img_bgr = new fbmp_image(job_state.image_width, job_state.image_height);
    }
    job_state.can_partial_update = false;
  }

  void rcpu::render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera_config& in_camera_config)
  {
    if (job_state.is_working) return;
    
    set_job_state(in_scene, in_renderer_config, in_camera_config);

    job_state.is_working = true;
    if(worker_thread == nullptr)
    {
      worker_thread = new std::thread(&rcpu::worker_function, this);
      worker_semaphore = new std::binary_semaphore(0);
    }
    worker_semaphore->release();
  }

  void rcpu::push_partial_update()
  {
    if(!job_state.can_partial_update) return;
    
    fdx11& dx = fdx11::instance();
    if (output_texture)
    {
      dx.update_texture_buffer(get_img_rgb(), job_state.image_width, job_state.image_height, output_texture);
    }
  }

  void rcpu::worker_function()
  {
    while (true)
    {
      worker_semaphore->acquire();
      if (job_state.requested_stop) { break; }
      job_pre_update();
      job_update();
      job_post_update();
    }
  }

  void rcpu::job_pre_update()
  {
    fstats::reset();
    benchmark_render.start("Render");

    if(job_state.image_width != 0 && job_state.image_height != 0)
    {
      fdx11& dx = fdx11::instance();
      bool ret = dx.load_texture_from_buffer(get_img_rgb(), job_state.image_width, job_state.image_height, &output_srv, &output_texture);
      assert(ret);
    
      job_state.can_partial_update = true;
    }
  }

  void rcpu::job_post_update()
  {
    job_state.benchmark_render_time = benchmark_render.stop();
    job_state.is_working = false;
    
    job_state.ray_count = fstats::get_ray_count();
    job_state.ray_triangle_intersection_count = fstats::get_ray_triangle_intersection_count();
    job_state.ray_box_intersection_count = fstats::get_ray_box_intersection_count();
    job_state.ray_object_intersection_count = fstats::get_ray_object_intersection_count();
    
    if (save_output)
    {
      char image_file_name[300];  // Run-Time Check Failure #2 - Stack around the variable 'image_file_name' was corrupted.
      std::sprintf(image_file_name, fio::get_render_output_file_path().c_str());

      ftimer_instance benchmark_save;
      benchmark_save.start("Save");
      save(image_file_name);
      job_state.benchmark_save_time = benchmark_save.stop();
    }
  }
  
  void rcpu::save(const char* file_name) const
  {
    job_state.img_bgr->save_to_file(file_name);
  }
}
