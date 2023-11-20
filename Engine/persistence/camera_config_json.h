#pragma once

#include "persistence/serializable.h"
#include "math/vec3.h"
#include "math/camera.h"

namespace engine
{
  class ENGINE_API camera_config_serializer
  {
  public:
    static nlohmann::json serialize(const camera_config& value);
    static camera_config deserialize(const nlohmann::json& j);
  };
}
