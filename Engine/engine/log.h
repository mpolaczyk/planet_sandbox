#pragma once

#include "core/core.h"
#include <format>

namespace engine
{
  class ENGINE_API flogger
  {
  public:
    static void init();

    static void trace(const char* fmt);
    static void debug(const char* fmt);
    static void info(const char* fmt);
    static void warn(const char* fmt);
    static void error(const char* fmt);
    static void critical(const char* fmt);
    static void flush();
  };
}

#define LOG_TRACE(...) ::engine::flogger::trace(std::format(__VA_ARGS__).c_str());
#define LOG_DEBUG(...) ::engine::flogger::debug(std::format(__VA_ARGS__).c_str());
#define LOG_INFO(...) ::engine::flogger::info(std::format(__VA_ARGS__).c_str());
#define LOG_WARN(...) ::engine::flogger::warn(std::format(__VA_ARGS__).c_str());
#define LOG_ERROR(...) ::engine::flogger::error(std::format(__VA_ARGS__).c_str());
#define LOG_CRITICAL(...) ::engine::flogger::critical(std::format(__VA_ARGS__).c_str());