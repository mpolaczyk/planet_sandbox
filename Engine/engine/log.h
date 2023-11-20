#pragma once

#include "core/core.h"
#include <format>

namespace engine
{
  class ENGINE_API logger
  {
  public:
    static void init();
    
    static void trace(const char* fmt);
    static void debug(const char* fmt);
    static void info(const char* fmt);
    static void warn(const char* fmt);
    static void error(const char* fmt);
    static void critical(const char* fmt);
  };
}

#define LOG_TRACE(...) ::engine::logger::trace(std::format(__VA_ARGS__).c_str())
#define LOG_DEBUG(...) ::engine::logger::debug(std::format(__VA_ARGS__).c_str())
#define LOG_INFO(...) ::engine::logger::info(std::format(__VA_ARGS__).c_str())
#define LOG_WARN(...) ::engine::logger::warn(std::format(__VA_ARGS__).c_str())
#define LOG_ERROR(...) ::engine::logger::error(std::format(__VA_ARGS__).c_str())
#define LOG_CRITICAL(...) ::engine::logger::critical(std::format(__VA_ARGS__).c_str())