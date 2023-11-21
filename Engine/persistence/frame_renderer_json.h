#pragma once

#include "renderer/cpu_renderer_base.h"

#include "persistence/serializable.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API renderer_config_serializer
  {
  public:
    static nlohmann::json serialize(const renderer_config& value);
    static renderer_config deserialize(const nlohmann::json& j);
  };
}
