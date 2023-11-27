
#include "persistence/serializable.cpp"
#include "object/object_types.h"

namespace engine
{
  template bool ENGINE_API try_parse<bool>(const nlohmann::json& j, const std::string& key, bool& out_value, const char* function_name);
  template bool ENGINE_API try_parse<int>(const nlohmann::json& j, const std::string& key, int& out_value, const char* function_name);
  template bool ENGINE_API try_parse<float>(const nlohmann::json& j, const std::string& key, float& out_value, const char* function_name);
  template bool ENGINE_API try_parse<int32_t>(const nlohmann::json& j, const std::string& key, int32_t& out_value, const char* function_name);

  template bool ENGINE_API try_parse<object_type>(const nlohmann::json& j, const std::string& key, object_type& out_value, const char* function_name);

  template bool ENGINE_API try_parse<std::string>(const nlohmann::json& j, const std::string& key, std::string& out_value, const char* function_name);
  template bool ENGINE_API try_parse<nlohmann::json>(const nlohmann::json& j, const std::string& key, nlohmann::json& out_value, const char* function_name);
}