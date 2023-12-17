#pragma once

#include "nlohmann/json_fwd.hpp"

#include "core/core.h"
#include "math/vec3.h"

namespace engine
{
  class renderer_config;
  class camera_config;
  struct soft_asset_ptr_base;

  class ENGINE_API persistence
  {
  public:
    
    static nlohmann::json serialize(const soft_asset_ptr_base& value);
    static void deserialize(const nlohmann::json& j, soft_asset_ptr_base& out_value);

    static nlohmann::json serialize(const vec3& value);
    static void deserialize(const nlohmann::json& j, vec3& out_value);

    static nlohmann::json serialize(const camera_config& value);
    static void deserialize(const nlohmann::json& j, camera_config& out_value);

    static nlohmann::json serialize(const renderer_config& value);
    static void deserialize(const nlohmann::json& j, renderer_config& out_value);
    
  };
}