#pragma once

#include "core/application.h"
#include "core/windows_minimal.h"
#include "core/exceptions/windows_error.h"
#include "engine/unique_ptr.h"
#include "engine/window.h"
#include "engine/log.h"
#include "engine/string_tools.h"
#include "engine/renderer/dx12_lib.h"

using namespace engine;

namespace engine
{
  // Provide this in the user application
  extern fapplication* create_application();
  extern fwindow* create_window();
}

void inline main_impl(int argc, char** argv)
{
  flogger::init();
  if (argc == 1)
  {
    LOG_ERROR("Command line argument required: project name");
    return;
  }

  // Application lifecycle scope, don't remove brackets.
  {
    funique_ptr<fapplication> app;
    app.reset(create_application());
    fapplication::set_instance(app.get());
    app->set_window(create_window());
    app->init(argv[1]);
    app->main_loop();
  }
  
#ifdef BUILD_DEBUG
  // Notes:
  // 1. Everything owned by an application should be destroyed at this point.
  // 2. Command queue flushes GPU on destruction.
  fdx12::report_live_objects();
#endif
  
  LOG_INFO("Goodbye!");
}

int main(int argc, char** argv)
{
  if (!IsDebuggerPresent())
  {
    fwindows_error::set_all_exception_handlers();
  }
  main_impl(argc, argv);

  return 0;
}



