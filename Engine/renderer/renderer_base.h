#pragma once

#include <string>

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
  class ENGINE_API renderer_config
  {
  public:
    // Anti Aliasing oversampling
    int rays_per_pixel = 20;

    // Diffuse reflection
    int ray_bounces = 7;

    // How work is processed
    const class_object* type = nullptr;
    const class_object* new_type = nullptr;   // For UI
    
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
  class ENGINE_API renderer_base : public object
  {
  public:
    OBJECT_DECLARE(renderer_base, object)

    renderer_base() = default;
    virtual ~renderer_base();
    renderer_base(const renderer_base&) = default; // FIX w don't want to copy renderer but all objects are copyable... make it optional
    renderer_base& operator=(const renderer_base&) = default;
    renderer_base(renderer_base&&) = delete;
    renderer_base& operator=(renderer_base&&) = delete;
    
    // Output texture
    ID3D11ShaderResourceView* output_srv = nullptr;
    ID3D11Texture2D* output_texture = nullptr;
        
    // Main thread public interface.
    virtual void render_frame(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config) = 0;
    virtual void cancel() = 0;
    virtual bool is_working() const = 0;
    
  protected:
    virtual void cleanup();
  };
}
