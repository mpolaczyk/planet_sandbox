#pragma once

#include "nlohmann/json_fwd.hpp"

#include "core/core.h"

#define TRY_PARSE(t, j, key, out_value) engine::try_parse<t>(j, key, out_value, __FUNCTION__) 

namespace engine
{
  template<typename T>
  bool ENGINE_API try_parse(const nlohmann::json& j, const std::string& key, T& out_value, const char* function_name = nullptr);
}