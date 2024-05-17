#pragma once

#include <stdint.h>
#include <DirectXMath.h>

#include "core/core.h"
#include "math/vec3.h"

namespace engine
{
  using namespace DirectX;
  
  class ENGINE_API fcamera
  {
  public:
    fcamera() = default;
    fcamera(const fvec3& look_from, float field_of_view)
      : location(look_from), field_of_view(field_of_view)
    {
    }
    
    uint32_t get_hash() const;

    void update(float delta_time, int32_t width, int32_t height);
    
    // Camera movement
    int32_t input_forward, input_left, input_backward, input_right;
    int32_t input_down, input_up;
    int32_t input_yaw;
    int32_t input_pitch;
    float move_speed = 5.f;
    float rotate_speed = 5.f;
    
    // Runtime members
    XMFLOAT4X4 view_projection;
    XMFLOAT4 forward;
    XMFLOAT4 right;
    XMFLOAT4 up;
    
    // Persistent members
    fvec3 location;
    float pitch;  
    float yaw;
    float field_of_view = 70.0f;
  };
}