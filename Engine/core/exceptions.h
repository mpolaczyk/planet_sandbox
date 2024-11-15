#pragma once

#include <stdexcept>
#include <string>
#include <format>

#include "core/core.h"

#define THROW_IF_FAILED(code) if(FAILED(code)) throw engine::fhresult_exception(code, __FILE__, __LINE__, __FUNCTION__);

namespace engine
{
  struct ENGINE_API fwindows_error
  {
    static std::string get_hresult_description(HRESULT hr);
    static std::string get_seh_exception_call_stack(CONTEXT* in_context);
    static void throw_cpp_exception_from_seh_exception(unsigned int in_code, EXCEPTION_POINTERS* info);
    static void test_seh_exception();
    static void test_hresult_exception();
    static void test_cpp_exception();
  };
  
  class ENGINE_API fhresult_exception : public std::runtime_error
  {
  public:
    fhresult_exception(HRESULT in_code)
      : std::runtime_error("HRESULT exception"), code(in_code) {}
  
    fhresult_exception(HRESULT in_code, const std::string&& in_context)
      : std::runtime_error("HRESULT exception"), code(in_code), context(in_context) {}

    fhresult_exception(HRESULT in_code, const std::string& in_file, int in_line, const std::string& in_function)
      : std::runtime_error("HRESULT exception"), code(in_code)
    {
      context = std::format("{0} in {1} line {2}\n", in_function, in_file, in_line);
    }
    
    virtual char const* what() const override;
    
  private:
    HRESULT code;
    std::string context;
  };

  /* Based on
   https://stackoverflow.com/questions/3730654/whats-better-to-use-a-try-except-block-or-a-try-catch-block
   https://docs.microsoft.com/en-us/cpp/cpp/exception-handling-differences?view=msvc-170

   The major difference between C structured exception handling (SEH) and C++ exception handling is that the C++ exception handling model deals in types,
   while the C structured exception handling model deals with exceptions of one type; specifically, unsigned int.
   That is, C exceptions are identified by an unsigned integer value, whereas C++ exceptions are identified by data type.
  */

  class ENGINE_API fseh_exception : public std::runtime_error
  {
  private:
    CTOR_DEFAULT(fseh_exception)
    CTOR_MOVE_COPY_DELETE(fseh_exception)

  public:
    explicit fseh_exception(fseh_exception& e)
      : std::runtime_error("SEH exception"), code(e.code), context(e.context) { }

    explicit fseh_exception(unsigned int in_code, const std::string&& in_context)
      : std::runtime_error("SEH exception"), code(in_code), context(in_context) { }

    DTOR_DEFAULT(fseh_exception)
    
    virtual char const* what() const override;

  private:
    unsigned int code;
    std::string context;
  };
}
