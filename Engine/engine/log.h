#pragma once

#include "core/core.h"

#include "spdlog/spdlog.h"

namespace engine
{
  class ENGINE_API logger
  {
  public:
    static void init();
    inline static std::shared_ptr<spdlog::logger>& get_console_logger() { return s_console_logger; }

  private: 
    static std::shared_ptr<spdlog::logger> s_console_logger;
  };
}

#define LOG_TRACE(...) ::engine::logger::get_console_logger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::engine::logger::get_console_logger()->debug(__VA_ARGS__)
#define LOG_INFO(...) ::engine::logger::get_console_logger()->info(__VA_ARGS__)
#define LOG_WARN(...) ::engine::logger::get_console_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::engine::logger::get_console_logger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::engine::logger::get_console_logger()->critical(__VA_ARGS__)