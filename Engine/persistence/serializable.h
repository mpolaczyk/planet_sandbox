#pragma once

// Warning!
// Don't include serializable.h outside of "*_json.h" files. "nlohmann\json.hpp" is heavy to compile.

#include "core/core.h"

#include "nlohmann/json.hpp"

#define TRY_PARSE(t, j, key, out_value) engine::try_parse<t>(j, key, out_value, __FUNCTION__) 

namespace engine
{
  template<class T>
  class ENGINE_API serializable
  {
    virtual T serialize() = 0;
    virtual void deserialize(const T& payload) = 0;
  };

  template<typename T>
  bool ENGINE_API try_parse(const nlohmann::json& j, const std::string& key, T& out_value, const char* function_name = nullptr);
}