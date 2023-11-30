#pragma once

#include "core/windows_minimal.h"
#include "core/exceptions.h"
#include "engine/log.h"

extern engine::application* engine::create_appliation();

int main(int argc, char** argv)
{
  auto app = engine::create_appliation();

  if (!IsDebuggerPresent())
  {
    // Register SEH exception catching when no debugger is present
    _set_se_translator(engine::seh_exception::handler);
  }
  try
  {
    app->run();
  }
  catch (const std::exception& e)
  {
    LOG_CRITICAL("Exception handler:");
    LOG_CRITICAL("{0}", e.what());
    __debugbreak();
    system("pause");
  }

  delete app;

}