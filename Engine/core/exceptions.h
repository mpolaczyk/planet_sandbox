#pragma once

#include <exception>
#include <string>

#include "core/core.h"

#define THROW_IF_FAILED(code) if(HRESULT hr = code < 0) throw std::runtime_error(fwin32_error::get_error_description(hr));

namespace engine
{
  /* Based on
   https://stackoverflow.com/questions/3730654/whats-better-to-use-a-try-except-block-or-a-try-catch-block
   https://docs.microsoft.com/en-us/cpp/cpp/exception-handling-differences?view=msvc-170

   The major difference between C structured exception handling (SEH) and C++ exception handling is that the C++ exception handling model deals in types,
   while the C structured exception handling model deals with exceptions of one type; specifically, unsigned int.
   That is, C exceptions are identified by an unsigned integer value, whereas C++ exceptions are identified by data type.
  */

  class ENGINE_API fseh_exception : public std::exception
  {
  public:
    static void handler(unsigned int exception_code, _EXCEPTION_POINTERS* exception_info);
    static int describe(_EXCEPTION_POINTERS* p_exp);

  private:
    fseh_exception() = default;

  public:
    explicit fseh_exception(fseh_exception& e) : exception_code(e.exception_code)
    {
    }

    explicit fseh_exception(unsigned int in_exception_code) : exception_code(in_exception_code)
    {
    }

    ~fseh_exception() = default;

    virtual char const* what() const override;

  private:
    unsigned int exception_code;
  };

  struct ENGINE_API fwin32_error
  {
    static std::string get_last_error_as_string();
    static std::string get_error_description(HRESULT hr);
  };
}
