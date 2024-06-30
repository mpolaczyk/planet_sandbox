#pragma once

#include "core/application.h"
#include "core/windows_minimal.h"
#include "core/exceptions.h"
#include "core/window.h"
#include "engine/log.h"

using namespace engine;

namespace engine
{
  // Provide this in the user application
  extern fapplication* create_application();
  extern fwindow* create_window();
}

int main(int argc, char** argv)
{
  fapplication* app = create_application();
  fapplication::instance = app;
  app->window.reset(create_window());

  if(!IsDebuggerPresent())
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
      app->main_loop();
      app->cleanup();
    }
  }
  catch(const std::exception& e)
  {
    LOG_CRITICAL("Exception handler:");
    LOG_CRITICAL("{0}", e.what());
    __debugbreak();
    system("pause");
  }
  fapplication::instance = nullptr;
  delete app;
  return 0;
}
