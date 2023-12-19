#include "math/camera.h"
#include "math/random.h"

#include "engine/hash.h"

#include <corecrt_math.h>

namespace engine
{
  camera_config camera_config::lerp(const camera_config& a, const camera_config& b, float f)
  {
    camera_config answer = a;
    answer.dist_to_focus = math::lerp_float(a.dist_to_focus, b.dist_to_focus, f);
    answer.type = math::lerp_float(a.type, b.type, f);
    return answer;
  }

  uint32_t camera_config::get_hash() const
  {
    uint32_t a = hash::combine(hash::get(look_from), hash::get(look_dir), hash::get(field_of_view), hash::get(aspect_ratio_h));
    uint32_t b = hash::combine(hash::get(aspect_ratio_w), hash::get(aperture), hash::get(dist_to_focus), hash::get(type));
    return hash::combine(a, b, a, hash::get(look_dir));
  }

  // Camera movement
  void camera_config::move_up(float speed)
  {
    look_from += vec3(0.0f, speed, 0.0f);
  }

  void camera_config::move_down(float speed)
  {
    look_from -= vec3(0.0f, speed, 0.0f);
  }

  void camera_config::move_forward(float speed)
  {
    look_from -= look_dir * speed;
  }

  void camera_config::move_backward(float speed)
  {
    look_from += look_dir * speed;
  }

  void camera_config::move_left(float speed)
  {
    vec3 left_dir = math::cross(look_dir, vec3(0.0f, 1.0f, 0.0f));
    look_from += left_dir * speed;
  }

  void camera_config::move_right(float speed)
  {
    vec3 left_dir = math::cross(look_dir, vec3(0.0f, 1.0f, 0.0f));
    look_from -= left_dir * speed;
  }

  void camera_config::rotate(float roll, float pitch)
  {
    look_dir = math::rotate_roll(look_dir, roll);  // because x is the vertical axis
    look_dir = math::rotate_pitch(look_dir, pitch);
  }


  void camera::configure(const camera_config& in_camera_config)
  {
    camera_conf = in_camera_config;

    float theta = math::degrees_to_radians(camera_conf.field_of_view);
    float h = (float)tan(theta / 2.0f);
    viewport_height = 2.0f * h;                       // viewport size at the distance 1
    viewport_width = camera_conf.aspect_ratio_w / camera_conf.aspect_ratio_h * viewport_height;

    const vec3 view_up(0.0f, 1.0f, 0.0f);
    w = math::normalize(camera_conf.look_dir);
    u = math::normalize(math::cross(view_up, w));
    v = math::cross(w, u);

    // Focus plane at origin (size of the frustum at the focus distance)
    f.horizontal = viewport_width * u * camera_conf.dist_to_focus;
    f.vertical = viewport_height * v * camera_conf.dist_to_focus;
    f.lower_left_corner = camera_conf.look_from - f.horizontal / 2.0f - f.vertical / 2.0f;

    // Camera plane at origin (proportional to type)
    c.horizontal = f.horizontal * camera_conf.type;
    c.vertical = f.vertical * camera_conf.type;
    c.lower_left_corner = camera_conf.look_from - c.horizontal / 2.0f - c.vertical / 2.0f;

    lens_radius = camera_conf.aperture / 2.0f;
  }

  ray camera::get_ray(float uu, float vv) const
  {
    vec3 rd = lens_radius * random_cache::in_unit_disk();
    vec3 offset = u * rd.x + v * rd.y;

    if (camera_conf.type == 0.0f)
    {
      // Shoot rays from the point to the focus plane - perspective camera
      vec3 fpo = f.get_point(uu, vv);                     // point on the focus plane at origin
      vec3 fpf = fpo - w * camera_conf.dist_to_focus;           // point on the focus plane at the focus distance forward camera
      return ray(camera_conf.look_from - offset, math::normalize(fpf - camera_conf.look_from + offset));
    }
    else
    {
      // Don't shoot rays from the point, shoot from the plane that is proportionally smaller to focus plane
      vec3 cpo = c.get_point(uu, vv);             // point on the camera plane at origin
      vec3 fpo = f.get_point(uu, vv);               // point on the focus plane at origin
      vec3 fpf = fpo - w * camera_conf.dist_to_focus;     // point on the plane crossing frustum, forward camera
      return ray(cpo - offset, math::normalize(fpf - cpo + offset));
    }
  }

  uint32_t camera::get_hash()
  {
    return camera_conf.get_hash();
  }
}