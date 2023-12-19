﻿#include "stdafx.h"

#include "nlohmann/json.hpp"

#include "ui_persistence.h"

#include "app/ui/ui.h"

nlohmann::json ui_persistence::serialize(const window_config& value)
{
  nlohmann::json j;
  j["x"] = value.x;
  j["y"] = value.y;
  j["w"] = value.w;
  j["h"] = value.h;
  return j;
}

void ui_persistence::deserialize(const nlohmann::json& j, window_config& out_value)
{
  TRY_PARSE(int, j, "x", out_value.x);
  TRY_PARSE(int, j, "y", out_value.y);
  TRY_PARSE(int, j, "w", out_value.w);
  TRY_PARSE(int, j, "h", out_value.h);
}