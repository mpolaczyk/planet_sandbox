#pragma once

#include "nlohmann/json_fwd.hpp"

struct window_config;

class ui_persistence
{
public:
  static nlohmann::json serialize(const window_config& value);
  static void deserialize(const nlohmann::json& j, window_config& out_value);
};
