#pragma once

#include "core/windows_minimal.h"
#include "core/exceptions.h"
#include "engine/log.h"

using namespace engine;

namespace engine
{
  extern fapplication* create_application(); // Provide this in the user application
}

int main(int argc, char** argv)
{
  fapplication* app = create_application();
  fapplication::app_weak_ptr = app;
  
  if (!IsDebuggerPresent())
  {
    // Register SEH exception catching when no debugger is present
    _set_se_translator(fseh_exception::handler);
  }
  try
  {
    if(argc == 1)
    {
      throw new std::runtime_error("Application command line argument required: project name");
    }
    else
    {
      app->init(argv[1]);
      app->run();
      app->cleanup();
    }
  }
  catch (const std::exception& e)
  {
    LOG_CRITICAL("Exception handler:");
    LOG_CRITICAL("{0}", e.what());
    __debugbreak();
    system("pause");
  }
  fapplication::app_weak_ptr = nullptr;
  delete app;
  return 0;
}