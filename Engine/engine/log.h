#pragma once

#include "core/core.h"

namespace engine
{
  class ENGINE_API logger
  {
  public:
    static void init();
    
    static void trace(const char* fmt...);
    static void debug(const char* fmt...);
    static void info(const char* fmt...);
    static void warn(const char* fmt...);
    static void error(const char* fmt...);
    static void critical(const char* fmt...);
  };
}

#define LOG_TRACE(...) ::engine::logger::trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::engine::logger::debug(__VA_ARGS__)
#define LOG_INFO(...) ::engine::logger::info(__VA_ARGS__)
#define LOG_WARN(...) ::engine::logger::warn(__VA_ARGS__)
#define LOG_ERROR(...) ::engine::logger::error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::engine::logger::critical(__VA_ARGS__)