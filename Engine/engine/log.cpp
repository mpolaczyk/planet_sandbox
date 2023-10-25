#include "engine/log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace engine
{
  static std::shared_ptr<spdlog::logger> s_console_logger;

  void logger::init()
  {
    spdlog::set_pattern("[%H:%M:%S.%e] [thread %t] [%^%l%$] %v");

    s_console_logger = spdlog::stdout_color_mt("console");

#if BUILD_DEBUG
    s_console_logger->set_level(spdlog::level::trace);
#elif BUILD_RELEASE
    s_console_logger->set_level(spdlog::level::info);
#endif
  }

  void logger::trace(const char* fmt...)
  {
    s_console_logger->trace(fmt);
  }
  void logger::debug(const char* fmt...)
  {
    s_console_logger->debug(fmt);
  }
  void logger::info(const char* fmt...)
  {
    s_console_logger->info(fmt);
  }
  void logger::warn(const char* fmt...)
  {
    s_console_logger->warn(fmt);
  }
  void logger::error(const char* fmt...)
  {
    s_console_logger->error(fmt);
  }
  void logger::critical(const char* fmt...)
  {
    s_console_logger->critical(fmt);
  }

}
