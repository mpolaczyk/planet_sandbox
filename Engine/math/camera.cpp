#include "math/camera.h"
#include "math/random.h"

#include "engine/hash.h"

#include <corecrt_math.h>

namespace engine
{
  DirectX::XMVECTOR fcamera_config::default_forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
  DirectX::XMVECTOR fcamera_config::default_right = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
  DirectX::XMVECTOR fcamera_config::default_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  
  fcamera_config fcamera_config::lerp(const fcamera_config& a, const fcamera_config& b, float f)
  {
    fcamera_config answer = a;
    answer.dist_to_focus = fmath::lerp_float(a.dist_to_focus, b.dist_to_focus, f);
    answer.type = fmath::lerp_float(a.type, b.type, f);
    return answer;
  }

  uint32_t fcamera_config::get_hash() const
  {
    uint32_t a = fhash::combine(fhash::get(look_from), fhash::get(yaw), fhash::get(pitch), fhash::get(aspect_ratio_h));
    uint32_t b = fhash::combine(fhash::get(aspect_ratio_w), fhash::get(aperture), fhash::get(dist_to_focus), fhash::get(type));
    return fhash::combine(a, b, a, fhash::get(field_of_view));
  }

  void fcamera_config::update()
  {
    using namespace DirectX;
    
    projection = XMMatrixPerspectiveFovLH(field_of_view, aspect_ratio_w / aspect_ratio_h, fmath::t_min, fmath::infinity);
    rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, 0);

    forward = XMVector3Normalize(XMVector3TransformCoord(default_forward, rotation));
    right   = XMVector3Normalize(XMVector3TransformCoord(default_right, rotation));
    //up      = XMVector3Normalize(XMVector3TransformCoord(default_up, rotation));
    up      = XMVector3Normalize(XMVector3Cross(forward, right)); // this works
    
    const XMVECTOR eye_position = XMVectorSet(look_from.x, look_from.y, look_from.z, 0.f);
    const XMVECTOR focus_position = XMVectorAdd(eye_position, forward);

    view = XMMatrixLookAtLH(eye_position, focus_position, up);
  
    XMFLOAT3 forward2;
    XMStoreFloat3(&forward2, forward);
    look_dir = fvec3(forward2.x, forward2.y, forward2.z);

    /*
    cpu renderer - right handed
      horizontal x positive to the right
      vertical   y positive up (E key)
      depth      z positive from the screen
      pitch		   positive look up
      yaw        positive look left
      pitch=0, yaw=0 towards screen

    gpu renderer RH - 
      horizontal x positive to the left
      vertical   y positive down (E key)
      depth      z positive towards the screen
      pitch		   positive look up
      yaw        positive look left
      pitch=0, yaw=0 away from screen

    XMMatrixRotationRollPitchYaw(pitch, yaw, roll)
    XMMatrixRotationRollPitchYawFromVector([pitch, yaw, roll])
    roll  around z
    pitch around y
    yaw   around x
    */
  }
  
  // Camera movement
  void fcamera_config::move_up(float speed)
  {
    look_from += fvec3(0.0f, speed, 0.0f);
  }

  void fcamera_config::move_down(float speed)
  {
    look_from -= fvec3(0.0f, speed, 0.0f);
  }

  void fcamera_config::move_forward(float speed)
  {
    using namespace DirectX;
    XMFLOAT3 forward2;
    XMStoreFloat3(&forward2, forward);
    look_from -= look_dir * speed;
  }

  void fcamera_config::move_backward(float speed)
  {
    look_from += look_dir * speed;
  }

  void fcamera_config::move_left(float speed)
  {
    fvec3 left_dir = fmath::cross(look_dir, fvec3(0.0f, 1.0f, 0.0f));
    look_from += left_dir * speed;
  }

  void fcamera_config::move_right(float speed)
  {
    fvec3 left_dir = fmath::cross(look_dir, fvec3(0.0f, 1.0f, 0.0f));
    look_from -= left_dir * speed;
  }

  void fcamera_config::rotate(float delta_yaw, float delta_pitch)
  {
    pitch = pitch - delta_pitch;
    yaw = yaw - delta_yaw;
  }

  void fcamera::configure(const fcamera_config& in_camera_config)
  {
    camera_conf = in_camera_config;

    float theta = fmath::degrees_to_radians(camera_conf.field_of_view);
    float h = (float)tan(theta / 2.0f);
    viewport_height = 2.0f * h;                       // viewport size at the distance 1
    viewport_width = camera_conf.aspect_ratio_w / camera_conf.aspect_ratio_h * viewport_height;

    const fvec3 view_up(0.0f, 1.0f, 0.0f);
    w = fmath::normalize(camera_conf.look_dir);
    u = fmath::normalize(fmath::cross(view_up, w));
    v = fmath::cross(w, u);

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

  fray fcamera::get_ray(float uu, float vv) const
  {
    fvec3 rd = lens_radius * frandom_cache::in_unit_disk();
    fvec3 offset = u * rd.x + v * rd.y;

    if (camera_conf.type == 0.0f)
    {
      // Shoot rays from the point to the focus plane - perspective camera
      fvec3 fpo = f.get_point(uu, vv);                     // point on the focus plane at origin
      fvec3 fpf = fpo - w * camera_conf.dist_to_focus;           // point on the focus plane at the focus distance forward camera
      return fray(camera_conf.look_from - offset, fmath::normalize(fpf - camera_conf.look_from + offset));
    }
    else
    {
      // Don't shoot rays from the point, shoot from the plane that is proportionally smaller to focus plane
      fvec3 cpo = c.get_point(uu, vv);             // point on the camera plane at origin
      fvec3 fpo = f.get_point(uu, vv);               // point on the focus plane at origin
      fvec3 fpf = fpo - w * camera_conf.dist_to_focus;     // point on the plane crossing frustum, forward camera
      return fray(cpo - offset, fmath::normalize(fpf - cpo + offset));
    }
  }

  uint32_t fcamera::get_hash() const
  {
    return camera_conf.get_hash();
  }
}