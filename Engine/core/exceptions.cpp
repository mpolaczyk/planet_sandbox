
#include <signal.h>
#include "windows_minimal.h"
#include "dbghelp.h"
#pragma comment(lib,"dbghelp.lib")
#include <sstream>
#include <system_error>
#include <format>

#include "core/exceptions.h"

#include <exception>
#include <fstream>
#include <mutex>

#include "engine/io.h"
#include "engine/log.h"
#include "engine/string_tools.h"

// The goal of this code is to catch all possible exception types (hardware, seh, c++)
// Collect information, write log and minidump.
// Assumption: Error handlers will allocate memory, for now I don't care about OOM situations. No reason to pre-allocate.
// I also don't fork or handle this in a separate thread.

namespace engine
{
  namespace
  {
    std::mutex unhandled_exception_mutex;
  }
  
  void fwindows_error::set_all_exception_handlers()
  {
    // Based on https://stackoverflow.com/questions/13591334/what-actions-do-i-need-to-take-to-get-a-crash-dump-in-all-error-scenarios
    // Thank you pavel-p
    
    // Tell OS to:
    // SEM_FAILCRITICALERRORS - Don't display the critical-error-handler message box. Sends the error to the calling process.
    // SEM_NOGPFAULTERRORBOX - The system does not invoke Windows Error Reporting.
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

    // Supersede the top-level exception handler of each thread and process.
    SetUnhandledExceptionFilter(unknown_exception_handler); // can't use it inside try-catch block
    
    // Alternative! - but it requires to be in a try-catch block to work
    // Catch SEH exceptions and raise C++ typed exception. This needs to be done per thread!
    //_set_se_translator(throw_cpp_exception_from_seh_exception);
    
    // Handle exception when the CRT detects an invalid argument.
    _set_invalid_parameter_handler(invalid_parameter_handler);

    // Hander for the pure virtual function calls. Works per process.
    _set_purecall_handler(pure_virtual_call_handler);

    // Handle abort signal
    signal(SIGABRT, sig_abort_handler);
    _set_abort_behavior(0, 0);
    
    // Standard function
    std::set_terminate(std_terminate_handler);
  }

  void fwindows_error::invalid_parameter_handler(const wchar_t* expr, const wchar_t* func, const wchar_t* file, unsigned int line, uintptr_t reserved)
  {
    unknown_exception_handler();
  }

  void fwindows_error::pure_virtual_call_handler()
  {
    unknown_exception_handler();
  }

  void fwindows_error::sig_abort_handler(int sig)
  {
    // This is required, otherwise if there is another thread simultaneously tries to abort process will be terminated
    signal(SIGABRT, sig_abort_handler);
    unknown_exception_handler();
  }

  void fwindows_error::std_terminate_handler()
  {
    unknown_exception_handler();
  }
  
  LONG WINAPI fwindows_error::unknown_exception_handler(EXCEPTION_POINTERS* info)
  {
    // Prevent other threads of crash-reporting at the same time to avoid ragnarok
    //const std::lock_guard<std::mutex> lock(unhandled_exception_mutex);  // This causes: Error C2712 : Cannot use __try in functions that require object unwinding
    unhandled_exception_mutex.lock();
    
    if (info)
    {
      // Handle existing SEH exception
      handle_exception(0, info);
    }
    else
    {
      // Convert this unknown event into a SEH exception
      __try
      {
        RaiseException(EXCEPTION_BREAKPOINT, 0, 0, nullptr);
      }
      __except(handle_exception(0, GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
      {
      }
    }

    return 0;
  }

  void fwindows_error::handle_exception(unsigned int in_code, EXCEPTION_POINTERS* info)
  {
    // Put this in every thread to handle SEH exception: _set_se_translator(throw_cpp_exception_from_seh_exception);
    // Compile with: /EHa
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-se-translator?view=msvc-170
    // https://crashrpt.sourceforge.net/docs/html/exception_handling.html
    
    const PEXCEPTION_RECORD record = info->ExceptionRecord;
    const DWORD code = record->ExceptionCode;   // System code. See STATUS_* macros in winnt.h
    const PVOID address = record->ExceptionAddress;
    const PVOID program_counter = reinterpret_cast<PVOID>(info->ContextRecord->Rip);
    
    std::ostringstream oss;
    oss << "GLOBAL EXCEPTION HANDLER!\n";
    {
      char buffer[30];
      sprintf(buffer, "Code: 0x%08x, program counter: 0x%p, address: 0x%p\n", code, program_counter, address);
      oss << buffer;
    }
    oss << std::strerror(code) << "\n";
    
    oss << "Exception information:\n";
    for (uint32_t i = 0; i < record->NumberParameters; i++)
    {
      char buffer[200];
      sprintf(buffer, "%08Ix\n", record->ExceptionInformation[i]);
      oss << buffer;
    }
    
    oss << "Call stack:\n";
    oss << get_seh_exception_call_stack(info->ContextRecord);

    fwindows_error::mini_dump_write_dump(info);
    std::ofstream log_file("crash_log.txt");
    log_file <<oss.str();
    log_file.close();

    LOG_CRITICAL("{0}", oss.str())
    flogger::flush();
    ::MessageBox(nullptr, L"Check the working directory for logs and crash dump.", L"Application crashed", MB_APPLMODAL | MB_ICONERROR | MB_OK);
  }

  std::string fwindows_error::get_hresult_description(HRESULT hr)
  {
    std::ostringstream oss;
    {
      wchar_t* message = nullptr;
      const auto num_chars = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 0, nullptr);
      if(num_chars > 0)
      {
        oss << fstring_tools::to_utf8(message) << "\n";
      }
    }
    oss << std::system_category().message(hr);
    return oss.str();
  }

  std::string fwindows_error::get_seh_exception_call_stack(CONTEXT* in_context)
  {
    // Shameless copy (and modified) from:
    // https://stackoverflow.com/questions/22467604/how-can-you-use-capturestackbacktrace-to-capture-the-exception-stack-not-the-ca
    // Thank you! MSDN.WhiteKnight
    
    const int max_name_len = 256;
    const int max_frames = 30;
    HANDLE process;
    HANDLE thread;
    HMODULE h_module;
    STACKFRAME64 stack;
    DWORD64 displacement;
    DWORD disp;
    IMAGEHLP_LINE64 *line;
    std::ostringstream oss;

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    char module[max_name_len];
    PSYMBOL_INFO symbol_info = (PSYMBOL_INFO)buffer;

    // On x64, StackWalk64 modifies the context record, that could cause crashes, so we create a copy to prevent it
    CONTEXT context_copy;
    memcpy(&context_copy, in_context, sizeof(CONTEXT));
    memset(&stack, 0, sizeof(STACKFRAME64));

    process                = GetCurrentProcess();
    thread                 = GetCurrentThread();
    displacement           = 0;
#if !defined(_M_AMD64)
    stack.AddrPC.Offset    = (*in_context).Eip;
    stack.AddrPC.Mode      = AddrModeFlat;
    stack.AddrStack.Offset = (*in_context).Esp;
    stack.AddrStack.Mode   = AddrModeFlat;
    stack.AddrFrame.Offset = (*in_context).Ebp;
    stack.AddrFrame.Mode   = AddrModeFlat;
#endif

    // Load symbols
    SymInitialize(process, nullptr, 1);

#if defined(_M_AMD64)
    int image_file_machne = IMAGE_FILE_MACHINE_AMD64;
#else
    int image_file_machne = IMAGE_FILE_MACHINE_I386;
#endif
    
    for(int frame = 0; frame < max_frames; frame++)
    {
      if(!StackWalk64(image_file_machne, process, thread, &stack, &context_copy, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
      {
        break;
      }
      
      // Get symbol name for address
      symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
      symbol_info->MaxNameLen = MAX_SYM_NAME;
      SymFromAddr(process, (ULONG64)stack.AddrPC.Offset, &displacement, symbol_info);

      line = (IMAGEHLP_LINE64 *)malloc(sizeof(IMAGEHLP_LINE64));
      line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);       

      // Try to get line
      if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &disp, line))
      {
        oss << std::format("{0}: {1} in {2} line {3}\n", frame, symbol_info->Name, line->FileName, line->LineNumber);
      }
      else
      { 
        // Failed to get line, at least print symbol name and module
        h_module = nullptr;
        lstrcpyA(module, "");
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)(stack.AddrPC.Offset), &h_module);
        if(h_module != nullptr)
        {
          GetModuleFileNameA(h_module, module, max_name_len);
        }
        oss << std::format("{0}: {1} address 0x{2:x} in {3}\n", frame, symbol_info->Name, symbol_info->Address, module);
      }       

      free(line);
      line = nullptr;
    }
    return oss.str();
  }

  LONG fwindows_error::mini_dump_write_dump(EXCEPTION_POINTERS* info)
  {
    const std::string path = std::format("{}/crash_dump.dmp", fio::get_working_dir());
    const std::wstring wpath = fstring_tools::to_utf16(path);
    HANDLE dump_file_handle = CreateFile(wpath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (dump_file_handle != INVALID_HANDLE_VALUE)
    {
      MINIDUMP_EXCEPTION_INFORMATION dump_info;
      dump_info.ExceptionPointers = info;
      dump_info.ThreadId = GetCurrentThreadId();
      dump_info.ClientPointers = true;

      MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dump_file_handle, MiniDumpNormal, &dump_info, nullptr, nullptr);
      CloseHandle(dump_file_handle);
    }
    return EXCEPTION_EXECUTE_HANDLER;
  }

  void fwindows_error::test_seh_exception()
  {
    // Access violation
    char* p = NULL;
    p[0] = 0;
    printf(p);  
  }

  void fwindows_error::test_hresult_exception()
  {
    THROW_IF_FAILED(-10)
  }

  void fwindows_error::test_cpp_exception()
  {
    throw std::out_of_range("Array N out of range");
  }

  void fwindows_error::test_abort()
  {
    abort();
  }

  void fwindows_error::test_std_terminate()
  {
    std::terminate(); // if not handled, will call abort()
  }
  
  void fwindows_error::test_std_exit()
  {
    std::exit(-1);
  }

  char const* fhresult_exception::what() const
  {
    std::ostringstream oss;
    oss << "HR code: " << std::to_string(static_cast<uint32_t>(code)) << "\n";
    oss << fwindows_error::get_hresult_description(code) << "\n";
    oss << "Context: " << context;
    std::string str = oss.str();
    char* buff = new char[str.size() + 1];
    strcpy_s(buff, str.size() + 1, str.c_str());
    return buff;
  }

  char const* fseh_exception::what() const
  {
    std::ostringstream oss;
    oss << "SEH code: " << std::format("{:x}", code) << "\n";
    oss << "Context: " << context;
    std::string str = oss.str();
    char* buff = new char[str.size() + 1];
    strcpy_s(buff, str.size() + 1, str.c_str());
    return buff;
  }


}
