#pragma once

#include <exception>

#include "core/core.h"

namespace engine
{
  /* Based on
   https://stackoverflow.com/questions/3730654/whats-better-to-use-a-try-except-block-or-a-try-catch-block
   https://docs.microsoft.com/en-us/cpp/cpp/exception-handling-differences?view=msvc-170

   The major difference between C structured exception handling (SEH) and C++ exception handling is that the C++ exception handling model deals in types,
   while the C structured exception handling model deals with exceptions of one type; specifically, unsigned int.
   That is, C exceptions are identified by an unsigned integer value, whereas C++ exceptions are identified by data type.
  */

  class ENGINE_API seh_exception : public std::exception
  {
  public:
    static void handler(unsigned int exception_code, _EXCEPTION_POINTERS* exception_info);
    static int describe(_EXCEPTION_POINTERS* p_exp);

  private:
    seh_exception() = default;

  public:
    explicit seh_exception(seh_exception& e) : exception_code(e.exception_code) {}
    explicit seh_exception(unsigned int in_exception_code) : exception_code(in_exception_code) {}
    ~seh_exception() = default;

    virtual char const* what() const override;

  private:
    unsigned int exception_code;
  };
}
