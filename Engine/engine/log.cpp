#include "engine/log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace engine
{
  static std::shared_ptr<spdlog::logger> s_console_logger;

  void flogger::init()
  {
    spdlog::set_pattern("[%H:%M:%S.%e] [t %t] [%^%l%$] %v");

    s_console_logger = spdlog::stdout_color_mt("console");

#if BUILD_DEBUG
    s_console_logger->set_level(spdlog::level::debug);
#elif BUILD_RELEASE
    s_console_logger->set_level(spdlog::level::info);
#endif
  }

#define FORMAT_LOG(text) std::format("[f {}] [{:.2f}ms] {}", g_frame_number, g_frame_time_ms, text)
  
  void flogger::trace(const char* fmt)
  {
    s_console_logger->trace(FORMAT_LOG(fmt));
  }

  void flogger::debug(const char* fmt)
  {
    s_console_logger->debug(FORMAT_LOG(fmt));
  }

  void flogger::info(const char* fmt)
  {
    s_console_logger->info(FORMAT_LOG(fmt));
  }

  void flogger::warn(const char* fmt)
  {
    s_console_logger->warn(FORMAT_LOG(fmt));
  }

  void flogger::error(const char* fmt)
  {
    s_console_logger->error(FORMAT_LOG(fmt));
  }

  void flogger::critical(const char* fmt)
  {
    s_console_logger->critical(FORMAT_LOG(fmt));
  }

  void flogger::flush()
  {
    s_console_logger->flush();
  }
}
