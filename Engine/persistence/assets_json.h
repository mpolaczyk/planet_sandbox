#pragma once

#include "core/core.h"

#include "persistence/serializable.h"

#include "assets/material.h"
#include "assets/mesh.h"
#include "assets/texture.h"
#include "asset/soft_asset_ptr.h"

namespace engine
{
  class ENGINE_API material_serializer
  {
  public:
    static nlohmann::json serialize(const material* value);
    static void deserialize(const nlohmann::json& j, material* out_value);
  };

  class ENGINE_API mesh_serializer
  {
  public:
    static nlohmann::json serialize(const mesh* value);
    static void deserialize(const nlohmann::json& j, mesh* out_value);
  };

  class ENGINE_API texture_serializer
  {
  public:
    static nlohmann::json serialize(const texture* value);
    static void deserialize(const nlohmann::json& j, texture* out_value);
  };

  class ENGINE_API soft_asset_ptr_base_serializer
  {
  public:
    static nlohmann::json serialize(const soft_asset_ptr_base* value);
    static void deserialize(const nlohmann::json& j, soft_asset_ptr_base* out_value);
  };
}