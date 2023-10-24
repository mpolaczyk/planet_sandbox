#include "log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace engine
{
  std::shared_ptr<spdlog::logger> logger::s_console_logger;

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

}
