#include "math/camera.h"
#include "math/random.h"

#include "engine/hash.h"

#include <corecrt_math.h>

namespace engine
{
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

  void fcamera_config::update(float delta_time)
  {
    using namespace DirectX;
    
    // Handle camera rotation
    {
      float rotate_ratio = rotate_speed * delta_time;
      yaw += static_cast<float>(input_yaw) * rotate_ratio;
      pitch += static_cast<float>(input_pitch) * rotate_ratio;
    }
    XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), 0);

    // Handle camera movement
    XMVECTOR translation = XMVectorSet(look_from.x, look_from.y, look_from.z, 1.f);
    {
      float move_ratio = move_speed * delta_time;
      XMVECTOR camera_translate = XMVectorSet(static_cast<float>(input_right - input_left) * move_ratio, 0.0f, static_cast<float>(input_forward - input_backward) * move_ratio, 1.0f * move_ratio);
      XMVECTOR camera_pan = XMVectorSet(0.0f, static_cast<float>(input_up - input_down) * move_ratio, 0.0f, 1.0f * move_ratio);
      translation += XMVector3Rotate(camera_translate, rotation);
      translation += camera_pan;
      translation = XMVectorSetW(translation, 1.0f);
    }
    XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(field_of_view), aspect_ratio_w / aspect_ratio_h, fmath::t_min, fmath::infinity);
    XMMATRIX rotation_matrix = XMMatrixTranspose(XMMatrixRotationQuaternion(rotation));
    XMMATRIX translation_matrix = XMMatrixTranslationFromVector(XMVectorNegate(translation));

    XMMATRIX view = translation_matrix * rotation_matrix;
    XMStoreFloat4x4(&view_projection, view * projection);

    XMFLOAT3 tr;                                    
    XMStoreFloat3(&tr, translation);                    
    look_from = fvec3(tr.x, tr.y, tr.z);

    // End of GPU part
    // Beginning of the CPU part
    {
      XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
      XMVector3Rotate(forward, rotation);
      XMFLOAT3 fwd;                                    
      XMStoreFloat3(&fwd, forward);      
      look_dir = fvec3(fwd.x, fwd.y, fwd.z);
    }
    
    float theta = fmath::degrees_to_radians(field_of_view);
    float h = (float)tan(theta / 2.0f);
    viewport_height = 2.0f * h;// * camera_conf.dist_to_focus;                       // viewport size at the distance 1
    viewport_width = (aspect_ratio_w / aspect_ratio_h) * viewport_height;

    const fvec3 view_up(0.0f, -1.0f, 0.0f);
    forward = fmath::normalize(-look_dir);
    right = fmath::normalize(fmath::cross(view_up, forward));
    up = fmath::cross(forward, right);

    // Focus plane at origin (size of the frustum at the focus distance)
    f.horizontal = viewport_width * right * dist_to_focus;
    f.vertical = viewport_height * up * dist_to_focus;
    f.lower_left_corner = look_from - f.horizontal / 2.0f - f.vertical / 2.0f;

    // Camera plane at origin (proportional to type)
    c.horizontal = f.horizontal * type;
    c.vertical = f.vertical * type;
    c.lower_left_corner = look_from - c.horizontal / 2.0f - c.vertical / 2.0f;

    lens_radius = aperture / 2.0f;
  }


  fray fcamera_config::get_ray(float uu, float vv) const
  {
    fvec3 rd = lens_radius * frandom_cache::in_unit_disk();
    fvec3 offset = right * rd.x + up * rd.y;

    if (type == 0.0f)
    {
      // Shoot rays from the point to the focus plane - perspective camera
      fvec3 fpo = f.get_point(uu, vv);                                 // point on the focus plane at origin
      fvec3 fpf = fpo - forward * dist_to_focus;           // point on the focus plane at the focus distance forward camera
      fvec3 ro = look_from - offset;
      fvec3 rd = fmath::normalize(fpf - look_from + offset);  // FIX! rd = 0 ?
      return fray(ro, rd);
    }
    else
    {
      // Don't shoot rays from the point, shoot from the plane that is proportionally smaller to focus plane
      fvec3 cpo = c.get_point(uu, vv);             // point on the camera plane at origin
      fvec3 fpo = f.get_point(uu, vv);               // point on the focus plane at origin
      fvec3 fpf = fpo - forward * dist_to_focus;     // point on the plane crossing frustum, forward camera
      return fray(cpo - offset, fmath::normalize(fpf - cpo + offset));
    }
  }
}