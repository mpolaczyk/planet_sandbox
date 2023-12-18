#pragma once

#include "core/core.h"

#include "math/vec3.h"
#include "math/plane.h"
#include "math/math.h"
#include <stdint.h>

namespace engine
{
  class ENGINE_API camera_config
  {
  public:
    camera_config() = default;
    camera_config(const vec3& look_from, const vec3& look_dir, float field_of_view, float aspect_ratio_w, float aspect_ratio_h, float aperture, float dist_to_focus, float type = 0.0f)
      : look_from(look_from), look_dir(look_dir), field_of_view(field_of_view), aspect_ratio_w(aspect_ratio_w), aspect_ratio_h(aspect_ratio_h), aperture(aperture), dist_to_focus(dist_to_focus), type(type)
    { }

    static camera_config lerp(const camera_config& a, const camera_config& b, float f);

    uint32_t get_hash() const;

    // Camera movement
    void move_up(float speed);
    void move_down(float speed);
    void move_forward(float speed);
    void move_backward(float speed);
    void move_left(float speed);
    void move_right(float speed);
    void rotate(float roll, float pitch);

    // Persistent members
    vec3 look_from;
    vec3 look_dir;
    float field_of_view = 90.0f;
    float aspect_ratio_h = 9.0f;
    float aspect_ratio_w = 16.0f;
    float aperture = 0.0f;       // defocus blur
    float dist_to_focus = 1.0f;  // distance from camera to the focus object
    float type = 0.0f;           // 0.0f perspective camera, 1.0f orthographic camera
  };

  class ENGINE_API camera
  {
  public:

    void configure(const camera_config& in_camera_config);

    ray inline get_ray(float uu, float vv) const;

    inline uint32_t get_hash();

  private:
    camera_config camera_conf;
    float lens_radius = 0.0f;
    float viewport_height = 2.0f;
    float viewport_width = 3.5f;
    plane f;  // focus plane at origin
    plane c;  // camera plane at origin
    vec3 w;   // back from camera vector
    vec3 u;   // right vector
    vec3 v;   // up vector
  };
}