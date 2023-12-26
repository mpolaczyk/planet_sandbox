#pragma once

#include "core/windows_minimal.h"
#include "core/exceptions.h"
#include "engine/log.h"

using namespace engine;

namespace engine
{
  extern application* create_application(); // Provide this in the user application
}

int main(int argc, char** argv)
{
  application* app = create_application();
  application::app_weak_ptr = app;
  
  if (!IsDebuggerPresent())
  {
    // Register SEH exception catching when no debugger is present
    _set_se_translator(seh_exception::handler);
  }
  try
  {
    app->init();
    app->run();
    app->cleanup();
  }
  catch (const std::exception& e)
  {
    LOG_CRITICAL("Exception handler:");
    LOG_CRITICAL("{0}", e.what());
    __debugbreak();
    system("pause");
  }
  application::app_weak_ptr = nullptr;
  delete app;
  return 0;
}