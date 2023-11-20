#pragma once

#include "app/app.h"
#include "math/camera.h"

#include "persistence/serializable.h"

#include "nlohmann/json_fwd.hpp"

class window_config_serializer
{
public:
  static nlohmann::json serialize(const window_config& value);
  static window_config deserialize(const nlohmann::json& j);
};

