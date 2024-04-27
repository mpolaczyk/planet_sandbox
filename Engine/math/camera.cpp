#include "math/camera.h"

#include "engine/hash.h"

#include <corecrt_math.h>

namespace engine
{
  uint32_t fcamera_config::get_hash() const
  {
    uint32_t a = fhash::combine(fhash::get(location), fhash::get(yaw), fhash::get(pitch), fhash::get(aspect_ratio_h));
    uint32_t b = fhash::combine(fhash::get(aspect_ratio_w), fhash::get(field_of_view));
    return fhash::combine(a, b);
  }

  void fcamera_config::update(float delta_time)
  {
    using namespace DirectX;
    const XMVECTOR _axis_forward = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
    const XMVECTOR _axis_up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
    
    // Handle camera rotation
    {
      float rotate_ratio = rotate_speed * delta_time;
      yaw += static_cast<float>(input_yaw) * rotate_ratio;
      pitch += static_cast<float>(input_pitch) * rotate_ratio;
    }
    XMVECTOR _rotation_quaternion = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), 0);

    // Handle camera movement
    XMVECTOR _location = XMVectorSet(location.x, location.y, location.z, 1.f);
    {
      float move_ratio = move_speed * delta_time;
      XMVECTOR _walk = XMVectorSet(static_cast<float>(input_right - input_left) * move_ratio, 0.0f, static_cast<float>(input_forward - input_backward) * move_ratio, 1.0f * move_ratio);
      XMVECTOR _climb = XMVectorSet(0.0f, static_cast<float>(input_up - input_down) * move_ratio, 0.0f, 1.0f * move_ratio);
      _location += XMVector3Rotate(_walk, _rotation_quaternion);
      _location += _climb;
      _location = XMVectorSetW(_location, 1.0f);
      XMFLOAT4 location2;
      XMStoreFloat4(&location2, _location);
      location.x = location2.x;
      location.y = location2.y;
      location.z = location2.z;
    }
    // Calculate camera
    {
      XMMATRIX _projection_matrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(field_of_view), aspect_ratio_w / aspect_ratio_h, fmath::t_min, fmath::infinity);
      XMMATRIX _rotation_matrix = XMMatrixTranspose(XMMatrixRotationQuaternion(_rotation_quaternion));
      XMMATRIX _translation_matrix = XMMatrixTranslationFromVector(XMVectorNegate(_location));
      XMMATRIX _view_matrix = _translation_matrix * _rotation_matrix;
      XMStoreFloat4x4(&view_projection, _view_matrix * _projection_matrix);
    }
    // Direction vectors
    {
      XMVECTOR _forward = XMVector3Rotate(_axis_forward, _rotation_quaternion);
      XMVECTOR _right = XMVector4Normalize(XMVector3Cross(_axis_up, _forward));
      XMVECTOR _up = XMVector3Cross(_forward, _right);
      XMStoreFloat4(&forward, _forward);
      XMStoreFloat4(&right, _right);
      XMStoreFloat4(&up, _up);
    }
  }
}