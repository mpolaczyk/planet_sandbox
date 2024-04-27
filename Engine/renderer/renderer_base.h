#pragma once

#include <string>
#include <wrl/client.h>

#include "resources/bmp.h"
#include "engine/hash.h"

#include "math/camera.h"
#include "hittables/hittables.h"
#include "hittables/scene.h"
#include "object/object.h"
#include "profile/benchmark.h"

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace engine
{
  class ENGINE_API frenderer_config
  {
  public:
    // Anti Aliasing oversampling
    int rays_per_pixel = 20;

    // Diffuse reflection
    int ray_bounces = 7;

    // How work is processed
    const oclass_object* type = nullptr;
    const oclass_object* new_type = nullptr;   // For UI
    
    // Draw in the same memory - real time update
    bool reuse_buffer = true;

    int resolution_vertical = 0;
    int resolution_horizontal = 0;

    // Manually set the brightest point of an image used for tone mapping
    float white_point = 1.0f;

    inline uint32_t get_hash() const
    {
      return fhash::combine(rays_per_pixel, ray_bounces, reuse_buffer, fhash::combine(resolution_vertical, resolution_horizontal, fhash::get(white_point), 1));
    }
  };

  // The responsibility of this class is to render to a texture
  class ENGINE_API rrenderer_base : public oobject
  {
  public:
    OBJECT_DECLARE(rrenderer_base, oobject)

    rrenderer_base() = default;
    rrenderer_base(const rrenderer_base&) = delete;
    rrenderer_base& operator=(const rrenderer_base&) = delete;
    rrenderer_base(rrenderer_base&&) = delete;
    rrenderer_base& operator=(rrenderer_base&&) = delete;

    fcamera_config camera;
    const hscene* scene = nullptr;
    
    // Output texture
    ComPtr<ID3D11Texture2D> output_texture;
    ComPtr<ID3D11ShaderResourceView> output_srv;
    ComPtr<ID3D11RenderTargetView> output_rtv;
    ComPtr<ID3D11DepthStencilView> output_dsv;
    ComPtr<ID3D11Texture2D> output_depth_texture;
    unsigned int output_width = 0;
    unsigned int output_height = 0;
    
    // Main public interface
    virtual void render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera_config& in_camera_config);
    float get_frame_time() const { return static_cast<float>(delta_time); };
    void start_frame_timer();
    void stop_frame_timer();
    
    // Async worker public interface
    virtual void push_partial_update() { };
    virtual void cancel() { };
    virtual bool is_async() const { return false; };
    virtual bool is_cancelled() const { return false; };
    bool is_working() const { return benchmark_renderer.is_working(); };

  protected:
    virtual void init() = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;

    int64_t timestamp_start = 0;
    int64_t perf_counter_frequency = 0;
    double delta_time = 0.0f;    // [s]
    ftimer_instance benchmark_renderer;
    
  private:
    bool init_done = false;
  };
}
