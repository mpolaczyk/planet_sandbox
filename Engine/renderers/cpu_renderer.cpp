
#include <thread>
#include <semaphore>
#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "renderers/cpu_renderer.h"

#include "engine/io.h"
#include "engine/log.h"
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

  void rcpu::create_output_texture(bool cleanup)
  {
    if (job_state.img_rgb != nullptr)
    {
      if (cleanup || !job_state.renderer_conf.reuse_buffer)
      {
        delete job_state.img_rgb;
        delete job_state.img_bgr;
        job_state.img_rgb = nullptr;
        job_state.img_bgr = nullptr;
      }
    }
    else
    {
      job_state.img_rgb = new fbmp_image(output_width, output_height);
      job_state.img_bgr = new fbmp_image(output_width, output_height);
    }
    
    fdx11& dx = fdx11::instance();
    dx.create_texture_from_buffer(get_job_buffer_rgb(), output_width, output_height, output_srv, output_texture);
  }

  void rcpu::render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera_config& in_camera_config)
  {
    if (job_state.is_working) return;
    if (in_renderer_config.resolution_vertical == 0 || in_renderer_config.resolution_horizontal == 0) return;
    
    rrenderer_base::render_frame(in_scene, in_renderer_config, in_camera_config);
    
    // Copy everything on purpose
    job_state.image_width = output_width;
    job_state.image_height = output_height;
    job_state.renderer_conf = in_renderer_config;
    job_state.scene_root = scene;
    job_state.scene_root_hash = scene->get_hash();
    job_state.camera = camera;
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
    if (output_texture)
    {
      fdx11& dx = fdx11::instance();
      dx.update_texture_from_buffer(get_job_buffer_rgb(), job_state.image_width, job_state.image_height, output_texture);
    }
  }

  void rcpu::worker_function()
  {
    while (true)
    {
      worker_semaphore->acquire();
      if (job_state.requested_stop) { break; }
      worker_job_pre_update();
      worker_job_update();
      worker_job_post_update();
    }
  }

  void rcpu::worker_job_pre_update()
  {
    fstats::reset();
    start_frame_timer();
  }

  void rcpu::worker_job_post_update()
  {
    stop_frame_timer();
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
