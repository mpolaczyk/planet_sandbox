#pragma once

#include "core/core.h"

#include "math/vec3.h"
#include "math/plane.h"
#include "math/math.h"
#include <stdint.h>

namespace engine
{
  class ENGINE_API fcamera_config
  {
  public:
    fcamera_config() = default;
    fcamera_config(const fvec3& look_from, const fvec3& look_dir, float field_of_view, float aspect_ratio_w, float aspect_ratio_h, float aperture, float dist_to_focus, float type = 0.0f)
      : look_from(look_from), look_dir(look_dir), field_of_view(field_of_view), aspect_ratio_w(aspect_ratio_w), aspect_ratio_h(aspect_ratio_h), aperture(aperture), dist_to_focus(dist_to_focus), type(type)
    { }

    static fcamera_config lerp(const fcamera_config& a, const fcamera_config& b, float f);

    uint32_t get_hash() const;

    void update();
    
    // Camera movement
    void move_up(float speed);
    void move_down(float speed);
    void move_forward(float speed);
    void move_backward(float speed);
    void move_left(float speed);
    void move_right(float speed);
    void rotate(float roll, float pitch);

    static DirectX::XMVECTOR default_forward;
    static DirectX::XMVECTOR default_right;
    static DirectX::XMVECTOR default_up;
    
    // Runtime members
    DirectX::XMVECTOR forward = default_forward;
    DirectX::XMVECTOR right = default_right;
    DirectX::XMVECTOR up = default_up;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX rotation;
    DirectX::XMMATRIX view;
    fvec3 look_dir;
    
    // Persistent members
    fvec3 look_from;
    float pitch;
    float yaw;
    float field_of_view = 70.0f;
    float aspect_ratio_h = 9.0f;
    float aspect_ratio_w = 16.0f;
    float aperture = 0.0f;       // defocus blur
    float dist_to_focus = 1.0f;  // distance from camera to the focus object
    float type = 0.0f;           // 0.0f perspective camera, 1.0f orthographic camera
  };

  class ENGINE_API fcamera
  {
  public:

    void configure(const fcamera_config& in_camera_config);

    fray inline get_ray(float uu, float vv) const;

    inline uint32_t get_hash() const;

  private:
    fcamera_config camera_conf;
    float lens_radius = 0.0f;
    float viewport_height = 2.0f;
    float viewport_width = 3.5f;
    fplane f;  // focus plane at origin
    fplane c;  // camera plane at origin
    fvec3 w;   // back from camera vector
    fvec3 u;   // right vector
    fvec3 v;   // up vector
  };
}