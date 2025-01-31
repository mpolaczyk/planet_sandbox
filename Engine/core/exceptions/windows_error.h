#pragma once

#include <stdexcept>
#include <string>

#include "core/core.h"

#define THROW_IF_FAILED(code) if(FAILED(code)) throw engine::fhresult_exception(code, __FILE__, __LINE__, __FUNCTION__);

namespace engine
{
  struct ENGINE_API fwindows_error
  {
    static void set_all_exception_handlers();

    // Handlers
    static void invalid_parameter_handler(const wchar_t* expr, const wchar_t* func, const wchar_t* file, unsigned int line, uintptr_t reserved);
    static void pure_virtual_call_handler();
    static void sig_abort_handler(int sig);
    static void std_terminate_handler();
    static LONG WINAPI unknown_exception_handler(EXCEPTION_POINTERS* info = nullptr);
    
    static void handle_exception(unsigned int in_code, EXCEPTION_POINTERS* info);

    // Other tools
    static void get_hresult_description(HRESULT hr, std::ostringstream& oss);
    static void get_seh_exception_call_stack(CONTEXT* in_context, std::ostringstream& oss);
    static void mini_dump_write_dump(EXCEPTION_POINTERS* info, std::ostringstream& oss);

    // Testing functions
    static void test_seh_exception();
    static void test_hresult_exception();
    static void test_cpp_exception();
    static void test_abort();
    static void test_std_terminate();
    static void test_std_exit();
  };
  
  class ENGINE_API fhresult_exception : public std::runtime_error
  {
  public:
    fhresult_exception(HRESULT in_code)
      : std::runtime_error("HRESULT exception"), code(in_code) {}
  
    fhresult_exception(HRESULT in_code, const std::string&& in_context)
      : std::runtime_error("HRESULT exception"), code(in_code), context(in_context) {}

    fhresult_exception(HRESULT in_code, const std::string& in_file, int in_line, const std::string& in_function);
    
    virtual char const* what() const override;
    
  private:
    HRESULT code;
    std::string context;
  };
}
