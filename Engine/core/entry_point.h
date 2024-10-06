#pragma once

#include "core/application.h"
#include "core/windows_minimal.h"
#include "core/exceptions.h"
#include "core/window.h"
#include "engine/log.h"
#include "engine/string_tools.h"

using namespace engine;

namespace engine
{
  // Provide this in the user application
  extern fapplication* create_application();
  extern fwindow* create_window();
}

//void on_terminate() 
//{
//  LOG_CRITICAL("Terminating!")
//  if (auto pe = std::current_exception())
//  {
//    try
//    {
//      std::rethrow_exception(pe);
//    }
//    catch (const std::exception& e)
//    {
//      //...
//    }
//  }
//  abort();
//}

void main_body(int argc, char** argv, fapplication* app)
{
  if (argc == 1)
  {
    throw std::runtime_error("Application command line argument required: project name");
  }
  else
  {
    app->init(argv[1]);
    app->main_loop();
    app->cleanup();
  }
}

void guarded_main(int argc, char** argv, fapplication* app)
{
  // Catch SEH exception and raise C++ typed exception. This needs to be done per thread!
  _set_se_translator(fwindows_error::throw_cpp_exception_from_seh_exception);

  try
  {
    main_body(argc, argv, app);
  }
  catch (const std::exception& e)
  {
    std::ostringstream oss;
    oss << "Global exception hadler!\n" << e.what();
    LOG_CRITICAL("{0}", oss.str())
    LOG_FLUSH

    // TODO
    // c++ exception: does not have any call stack information

    ::MessageBox(app->window->get_window_handle(), fstring_tools::to_utf16(oss.str()).c_str(), L"Error Message", MB_APPLMODAL | MB_ICONERROR | MB_OK);
  }
}

int main(int argc, char** argv)
{
  fapplication* app = create_application(); // TODO uniqueptr
  fapplication::instance = app;
  app->window.reset(create_window());

  //std::set_terminate(on_terminate); // TODO crash dumps? logs?

  if (IsDebuggerPresent())
  {
    // Don't guard code with try/catch block and error handling so that IDE can catch all exceptions
    main_body(argc, argv, app);
  }
  else
  {
    guarded_main(argc, argv, app);
  }

  fapplication::instance = nullptr;
  delete app;
  return 0;
}



