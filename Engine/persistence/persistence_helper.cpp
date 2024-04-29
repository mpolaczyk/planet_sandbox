
#include "nlohmann/json.hpp"

#include "persistence/persistence_helper.h"
#include "engine/log.h"

namespace engine
{
  template<typename T>  
  bool try_parse(const nlohmann::json& j, const std::string& key, T& out_value, const char* function_name)
  {
    if (j.contains(key))
    {
      out_value = j[key];
      return true;
    }
    if (function_name != nullptr)
    {
      LOG_WARN("json try_parse key missing: {0} in function {1}", key, function_name);
    }
    else
    {
      LOG_WARN("json try_parse key missing: {0}", key);
    }
    return false;
  }
}