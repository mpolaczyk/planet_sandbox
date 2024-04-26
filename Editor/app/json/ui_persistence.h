#pragma once

#include "nlohmann/json_fwd.hpp"

namespace editor
{
  struct fwindow_config;

  class fui_persistence
  {
  public:
    static nlohmann::json serialize(const fwindow_config& value);
    static void deserialize(const nlohmann::json& j, fwindow_config& out_value);
  };
}