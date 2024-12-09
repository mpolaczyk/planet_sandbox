#include "math/camera.h"

#include "math.h"
#include "engine/hash.h"

namespace engine
{
  uint32_t fcamera::get_hash() const
  {
    uint32_t y = fhash::combine(fhash::get(location), fhash::get(yaw), fhash::get(pitch), fhash::get(field_of_view));
    return fhash::combine(y, fhash::get(view_projection), 1, 1);
  }

  void fcamera::update(float delta_time, int32_t width, int32_t height)
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
      XMVECTOR _walk = XMVectorSet(
        static_cast<float>(input_right - input_left) * move_ratio,
        0.0f,
        static_cast<float>(input_forward - input_backward) * move_ratio,
        1.0f * move_ratio);
      XMVECTOR _climb = XMVectorSet(
        0.0f,
        static_cast<float>(input_up - input_down) * move_ratio,
        0.0f,
        1.0f * move_ratio);
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
      XMMATRIX _projection_matrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(field_of_view), static_cast<float>(width) / static_cast<float>(height), fmath::t_min, fmath::infinity);
      XMMATRIX _rotation_matrix = XMMatrixTranspose(XMMatrixRotationQuaternion(_rotation_quaternion));
      XMMATRIX _translation_matrix = XMMatrixTranslationFromVector(XMVectorNegate(_location));
      XMMATRIX _view_matrix = _translation_matrix * _rotation_matrix;
      XMStoreFloat4x4(&view_projection, _view_matrix * _projection_matrix);
      XMStoreFloat4x4(&view, _view_matrix);
      XMStoreFloat4x4(&projection, _projection_matrix);
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

  fray fcamera::get_screen_space_ray(uint32_t ss_width, uint32_t ss_height, uint32_t ss_x, uint32_t ss_y) const
  {
    // Returns a ray in world space, based on screen space coordinates
    // Based on: https://antongerdelan.net/opengl/raycasting.html

    constexpr float axis_z_forward = 1.0f;
    
    // Normalised device coordinates
    float ndc_x = (ss_x * 2.0f) / static_cast<float>(ss_width) - 1.0f;
    float ndc_y = 1.0f - (2.0f * ss_y) / static_cast<float>(ss_height);

    // Homogenous clip coordinates
    XMVECTOR ray_clip = XMVectorSet(ndc_x, ndc_y, axis_z_forward, 1.f);

    // Camera coordinates
    XMMATRIX _projection = XMLoadFloat4x4(&projection);
    XMMATRIX _inverse_projection = XMMatrixInverse(nullptr, _projection);
    XMVECTOR _ray_eye = XMVector4Transform(ray_clip, _inverse_projection);
    _ray_eye.m128_f32[2] = axis_z_forward;
    _ray_eye.m128_f32[3] = 0.0f;

    // World coordinates
    XMMATRIX _view = XMLoadFloat4x4(&view);
    XMMATRIX _inverse_view = XMMatrixInverse(nullptr, _view);
    XMVECTOR _ray_world = XMVector4Transform(_ray_eye, _inverse_view);
    _ray_world = XMVector4Normalize(_ray_world);
    XMFLOAT4 ray_world2;
    XMStoreFloat4(&ray_world2, _ray_world);
    
    return fray(location, fvec3(ray_world2.x, ray_world2.y, ray_world2.z));
  }
}
