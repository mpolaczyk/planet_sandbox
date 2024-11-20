#pragma once

#include "core/application.h"
#include "core/windows_minimal.h"
#include "core/exceptions.h"
#include "core/window.h"
#include "engine/log.h"
#include "engine/string_tools.h"
#include "renderer/dx12_lib.h"

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
  if (argc == 0)
  {
    LOG_ERROR("Command line argument required: project name");
    return;
  }

  // Application lifecycle scope
  {
    std::shared_ptr<fapplication> app;
    app.reset(create_application());
    fapplication::get_instance() = app.get();
    app->set_window(create_window());
    app->init(argv[1]);
    app->main_loop();
    fapplication::get_instance() = nullptr;
  }
  
#ifdef BUILD_DEBUG
  fdx12::report_live_objects();
#endif
  
  LOG_INFO("Goodbye!");
}

void inline guarded_main_impl(int argc, char** argv)
{
  // Catch SEH exception and raise C++ typed exception. This needs to be done per thread!
  _set_se_translator(fwindows_error::throw_cpp_exception_from_seh_exception);

  try
  {
    main_impl(argc, argv);
  }
  catch (const std::exception& e)
  {
    std::ostringstream oss;
    oss << "Global exception hadler!\n" << e.what();
    LOG_CRITICAL("{0}", oss.str())
    flogger::flush();
    ::MessageBox(nullptr, fstring_tools::to_utf16(oss.str()).c_str(), L"Error Message", MB_APPLMODAL | MB_ICONERROR | MB_OK);
  }
}

int main(int argc, char** argv)
{
  // Don't guard code with try/catch block and error handling so that IDE can catch all exceptions
  if (IsDebuggerPresent())
  {
    main_impl(argc, argv);
  }
  else
  {
    guarded_main_impl(argc, argv);
  }
  return 0;
}



