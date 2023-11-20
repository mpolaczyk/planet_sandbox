#pragma once

#include "persistence/serializable.h"
#include "math/vec3.h"

namespace engine
{
  class ENGINE_API vec3_serializer
  {
  public:
    static nlohmann::json serialize(const vec3& value);
    static vec3 deserialize(const nlohmann::json& j);
  };
}
