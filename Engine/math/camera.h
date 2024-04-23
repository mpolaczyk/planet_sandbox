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

    void update(float delta_time);

    fray inline get_ray(float uu, float vv) const;
    
    // Camera movement
    int32_t input_forward, input_left, input_backward, input_right;
    int32_t input_down, input_up;
    float move_speed = 15.f;
    int32_t input_yaw;
    int32_t input_pitch;
    float rotate_speed = 10.f;
    
    // GPU renderer runtime members
    DirectX::XMFLOAT4X4 view_projection;
    
    // CPU renderer runtime members
    float lens_radius = 0.0f;
    float viewport_height = 2.0f;
    float viewport_width = 3.5f;
    fplane f;  // focus plane at origin
    fplane c;  // camera plane at origin
    fvec3 forward;   // forward vector
    fvec3 right;   // right vector
    fvec3 up;   // up vector
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
}