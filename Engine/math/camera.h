#pragma once

#include "core/core.h"

#include "math/vec3.h"
#include "math/math.h"
#include <stdint.h>

namespace engine
{
  class ENGINE_API fcamera_config
  {
  public:
    fcamera_config() = default;
    fcamera_config(const fvec3& look_from, float field_of_view, float aspect_ratio_w, float aspect_ratio_h)
      : location(look_from), field_of_view(field_of_view), aspect_ratio_w(aspect_ratio_w), aspect_ratio_h(aspect_ratio_h)
    {
    }
    
    uint32_t get_hash() const;

    void update(float delta_time);
    
    // Camera movement
    int32_t input_forward, input_left, input_backward, input_right;
    int32_t input_down, input_up;
    int32_t input_yaw;
    int32_t input_pitch;
    float move_speed = 10.f;
    float rotate_speed = 10.f;
    
    // Runtime members
    DirectX::XMFLOAT4X4 view_projection;
    DirectX::XMFLOAT4 forward;
    DirectX::XMFLOAT4 right;
    DirectX::XMFLOAT4 up;
    
    // Persistent members
    fvec3 location;
    float pitch;  
    float yaw;
    float field_of_view = 70.0f;
    float aspect_ratio_h = 9.0f;
    float aspect_ratio_w = 16.0f;
  };
}