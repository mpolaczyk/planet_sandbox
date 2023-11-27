#pragma once

#include "persistence/serializable.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"
#include "hittables/scene.h"
#include "hittables/hittables.h"

namespace engine
{
  class ENGINE_API hittable_serializer
  {
  public:
    static nlohmann::json serialize(const hittable* value);
    static void deserialize(const nlohmann::json& j, hittable* out_value);
  };

  class ENGINE_API scene_serializer
  {
  public:
    static nlohmann::json serialize(const scene* value);
    static void deserialize(const nlohmann::json& j, scene* out_value);
  };

  class ENGINE_API static_mesh_serializer
  {
  public:
    static nlohmann::json serialize(const static_mesh* value);
    static void deserialize(const nlohmann::json& j, static_mesh* out_value);
  };

  class ENGINE_API sphere_serializer
  {
  public:
    static nlohmann::json serialize(const sphere* value);
    static void deserialize(const nlohmann::json& j, sphere* out_value);
  };
}