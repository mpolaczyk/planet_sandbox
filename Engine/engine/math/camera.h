#pragma once

#include <stdint.h>
#include <DirectXMath.h>

#include "core/core.h"
#include "engine/math/ray.h"
#include "engine/math/vec3.h"

namespace engine
{
  using namespace DirectX;

  class ENGINE_API fcamera
  {
  public:
    CTOR_DEFAULT(fcamera)
    fcamera(const fvec3& look_from, float field_of_view)
      : location(look_from), field_of_view(field_of_view)
    {
    }

    uint32_t get_hash() const;

    void update(float delta_time, int32_t width, int32_t height);
    fray get_screen_space_ray(uint32_t ss_width, uint32_t ss_height, uint32_t ss_x, uint32_t ss_y) const;

    // Camera movement
    int32_t input_forward = 0;
    int32_t input_left = 0;
    int32_t input_backward = 0;
    int32_t input_right = 0;
    int32_t input_down = 0;
    int32_t input_up = 0;
    int32_t input_yaw = 0;
    int32_t input_pitch = 0;
    float move_speed = 15.f;
    float rotate_speed = 7.f;

    // Runtime members
    XMFLOAT4X4 view_projection{};
    XMFLOAT4X4 view{};
    XMFLOAT4X4 projection{};
    XMFLOAT4 forward{};
    XMFLOAT4 right{};
    XMFLOAT4 up{};

    // Persistent members
    fvec3 location{};
    float pitch = 0.0f;
    float yaw = 0.0f;
    float field_of_view = 70.0f;
  };
}
