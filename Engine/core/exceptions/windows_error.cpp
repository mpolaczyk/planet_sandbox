
#include <signal.h>
#include "core/windows_minimal.h"
#include "dbghelp.h"
#pragma comment(lib,"dbghelp.lib")
#include <sstream>
#include <system_error>
#include <format>

#include "core/exceptions/windows_error.h"

#include <exception>
#include <fstream>
#include <mutex>

#include "engine/io.h"
#include "engine/log.h"
#include "engine/string_tools.h"

/*
   The goal of this code is to catch all possible exception types (hardware, seh, c++)
   Collect information, write log and minidump.
   Assumption: Error handlers will allocate memory, for now I don't care about OOM situations. No reason to pre-allocate.
   I also don't fork or handle this in a separate thread.

   Note:
   https://stackoverflow.com/questions/3730654/whats-better-to-use-a-try-except-block-or-a-try-catch-block
   https://docs.microsoft.com/en-us/cpp/cpp/exception-handling-differences?view=msvc-170

   The major difference between C structured exception handling (SEH) and C++ exception handling is that the C++ exception handling model deals in types,
   while the C structured exception handling model deals with exceptions of one type; specifically, unsigned int.
   That is, C exceptions are identified by an unsigned integer value, whereas C++ exceptions are identified by data type.
  */
namespace engine
{  
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

  namespace
  {
    std::mutex unhandled_exception_mutex;
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

  namespace
  {
    constexpr int call_stack_max_name_len = 1024;
    constexpr int call_stack_max_frames = 100;
    struct handle_exception_preallocation
    {
      handle_exception_preallocation()
      {
        message.reserve(1024 + call_stack_max_name_len * call_stack_max_frames);
        oss = std::ostringstream(message);
      }
      ~handle_exception_preallocation() = default;
      
      std::string message{};
      std::ostringstream oss{};
      PEXCEPTION_RECORD record{};
      DWORD code{};
      PVOID address{};
      PVOID program_counter{};  
    } he_data;
  }
  void fwindows_error::handle_exception(unsigned int in_code, EXCEPTION_POINTERS* info)
  {
    // Put this in every thread to handle SEH exception: _set_se_translator(throw_cpp_exception_from_seh_exception);
    // Compile with: /EHa
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-se-translator?view=msvc-170
    // https://crashrpt.sourceforge.net/docs/html/exception_handling.html
    
    he_data.record = info->ExceptionRecord;
    he_data.code = he_data.record->ExceptionCode;   // System code. See STATUS_* macros in winnt.h
    he_data.address = he_data.record->ExceptionAddress;
    he_data.program_counter = reinterpret_cast<PVOID>(info->ContextRecord->Rip);
    
    he_data.oss << "GLOBAL EXCEPTION HANDLER\n";
    he_data.oss << std::format("Error code: {} program counter: {} address: {}\n", he_data.code, he_data.program_counter, he_data.address);
    he_data.oss << "Message: " << std::strerror(he_data.code) << "\n";
    he_data.oss << "Exception information: ";
    for (uint32_t i = 0; i < he_data.record->NumberParameters; i++)
    {
      he_data.oss << std::format("{} ", he_data.record->ExceptionInformation[i]);
    }
    he_data.oss << "\n";
    
    get_seh_exception_call_stack(info->ContextRecord, he_data.oss);

    mini_dump_write_dump(info, he_data.oss);
    
    std::ofstream log_file("crash_log.txt");
    log_file << he_data.oss.str();
    log_file.close();

    ::MessageBox(nullptr, L"Check the working directory for logs and crash dump.", L"Application crashed", MB_APPLMODAL | MB_ICONERROR | MB_OK);

    LOG_CRITICAL("{0}", he_data.oss.str())
    flogger::flush();
  }

  namespace
  {
    struct get_hresult_description_preallocation
    {
      wchar_t* message = nullptr;
      DWORD num_chars{};
    } ghdp_data;
  }
  void fwindows_error::get_hresult_description(HRESULT hr, std::ostringstream& oss)
  {
    ghdp_data.num_chars = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&ghdp_data.message, 0, nullptr);
    if(ghdp_data.num_chars > 0)
    {
      oss << fstring_tools::to_utf8(ghdp_data.message) << "\n";
    }
    oss << std::system_category().message(hr);
  }
  
  namespace
  {
    struct get_seh_exception_call_stack_preallocation
    {
      char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)]{};
      char module[call_stack_max_name_len]{};
      HANDLE process{};
      HANDLE thread{};
      HMODULE h_module{};
      STACKFRAME64 stack{};
      DWORD64 displacement{};
      DWORD disp{};
      IMAGEHLP_LINE64 line{};
      PSYMBOL_INFO symbol_info{};
      CONTEXT context_copy{};
      int image_file_machne=0;
      int frame=0;
    } gsecsd_data;
  }
  void fwindows_error::get_seh_exception_call_stack(CONTEXT* in_context, std::ostringstream& oss)
  {
    // Shameless copy (and modified) from:
    // https://stackoverflow.com/questions/22467604/how-can-you-use-capturestackbacktrace-to-capture-the-exception-stack-not-the-ca
    // Thank you! MSDN.WhiteKnight
    oss << "Call stack:\n";
    
    gsecsd_data.symbol_info = (PSYMBOL_INFO)gsecsd_data.buffer;

    // On x64, StackWalk64 modifies the context record, that could cause crashes, so we create a copy to prevent it
    memcpy(&gsecsd_data.context_copy, in_context, sizeof(CONTEXT));
    memset(&gsecsd_data.stack, 0, sizeof(STACKFRAME64));

    gsecsd_data.process                = GetCurrentProcess();
    gsecsd_data.thread                 = GetCurrentThread();
    gsecsd_data.displacement           = 0;
#if !defined(_M_AMD64)
    gsecsd_data.stack.AddrPC.Offset    = (*in_context).Eip;
    gsecsd_data.stack.AddrPC.Mode      = AddrModeFlat;
    gsecsd_data.stack.AddrStack.Offset = (*in_context).Esp;
    gsecsd_data.stack.AddrStack.Mode   = AddrModeFlat;
    gsecsd_data.stack.AddrFrame.Offset = (*in_context).Ebp;
    gsecsd_data.stack.AddrFrame.Mode   = AddrModeFlat;
#endif
    // Load symbols
    SymInitialize(gsecsd_data.process, nullptr, 1);
#if defined(_M_AMD64)
    gsecsd_data.image_file_machne = IMAGE_FILE_MACHINE_AMD64;
#else
    gsecsd_data.image_file_machne = IMAGE_FILE_MACHINE_I386;
#endif
    
    for(gsecsd_data.frame = 0; gsecsd_data.frame < call_stack_max_frames; gsecsd_data.frame++)
    {
      if(!StackWalk64(gsecsd_data.image_file_machne, gsecsd_data.process, gsecsd_data.thread, &gsecsd_data.stack, &gsecsd_data.context_copy, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
      {
        break;
      }
      
      // Get symbol name for address
      gsecsd_data.symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
      gsecsd_data.symbol_info->MaxNameLen = MAX_SYM_NAME;
      SymFromAddr(gsecsd_data.process, (ULONG64)gsecsd_data.stack.AddrPC.Offset, &gsecsd_data.displacement, gsecsd_data.symbol_info);

      gsecsd_data.line = {};
      gsecsd_data.line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);       

      // Try to get line
      if (SymGetLineFromAddr64(gsecsd_data.process, gsecsd_data.stack.AddrPC.Offset, &gsecsd_data.disp, &gsecsd_data.line))
      {
        oss << std::format("{0}: {1} in {2} line {3}\n", gsecsd_data.frame, gsecsd_data.symbol_info->Name, gsecsd_data.line.FileName, gsecsd_data.line.LineNumber);
      }
      else
      { 
        // Failed to get line, at least print symbol name and module
        gsecsd_data.h_module = nullptr;
        lstrcpyA(gsecsd_data.module, "");
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)(gsecsd_data.stack.AddrPC.Offset), &gsecsd_data.h_module);
        if(gsecsd_data.h_module != nullptr)
        {
          GetModuleFileNameA(gsecsd_data.h_module, gsecsd_data.module, call_stack_max_name_len);
        }
        oss << std::format("{0}: {1} address 0x{2:x} in {3}\n", gsecsd_data.frame, gsecsd_data.symbol_info->Name, gsecsd_data.symbol_info->Address, gsecsd_data.module);
      }       
    }
  }

  namespace
  {
    struct mini_dump_write_dump_preallocation
    {
      mini_dump_write_dump_preallocation()
      {
        wpath.reserve(MAX_PATH);
      }
      ~mini_dump_write_dump_preallocation() = default;
      
      std::wstring wpath{};
      HANDLE dump_file_handle{};
      MINIDUMP_EXCEPTION_INFORMATION dump_info{};
    } mdwd_data;
  }
  void fwindows_error::mini_dump_write_dump(EXCEPTION_POINTERS* info, std::ostringstream& oss)
  {
    mdwd_data.wpath = L"crash_dump.dmp";
    mdwd_data.dump_file_handle = CreateFile(mdwd_data.wpath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (mdwd_data.dump_file_handle != INVALID_HANDLE_VALUE)
    {
      mdwd_data.dump_info.ExceptionPointers = info;
      mdwd_data.dump_info.ThreadId = GetCurrentThreadId();
      mdwd_data.dump_info.ClientPointers = true;

      if(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), mdwd_data.dump_file_handle, MiniDumpNormal, &mdwd_data.dump_info, nullptr, nullptr))
      {
        oss << "MiniDumpWriteDump succedded";
      }
      else
      {
        oss << "MiniDumpWriteDump failed!";
      }
      CloseHandle(mdwd_data.dump_file_handle);
    }
    else
    {
      oss << "Invalid crash dump handle!";
    }
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
    fwindows_error::get_hresult_description(code, oss);
    oss << "Context: " << context;
    std::string str = oss.str();
    char* buff = new char[str.size() + 1];
    strcpy_s(buff, str.size() + 1, str.c_str());
    return buff;
  }
}
