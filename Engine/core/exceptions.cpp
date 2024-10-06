#include "windows_minimal.h"
#include "dbghelp.h"
#pragma comment(lib,"dbghelp.lib")
#include <sstream>
#include <system_error>
#include <format>

#include "core/exceptions.h"
#include "engine/string_tools.h"

// Assumption: Error handlers will allocate memory, for now I don't care about OOM situations. No reason to pre-allocate.

namespace engine
{
  std::string fwindows_error::get_hresult_description(HRESULT hr)
  {
    std::ostringstream oss;
    {
      wchar_t* message = nullptr;
      const auto num_chars = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                                         nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 0, nullptr);
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
    const int max_frames = 10;
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
  
  void fwindows_error::throw_cpp_exception_from_seh_exception(unsigned int in_code, EXCEPTION_POINTERS* info)
  {
    if(IsDebuggerPresent())
    {
      __debugbreak();
      system("pause");
    }
    
    // Put this in every thread to handle SEH exception: _set_se_translator(throw_cpp_exception_from_seh_exception);
    // Compile with: /EHa
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-se-translator?view=msvc-170
    // https://crashrpt.sourceforge.net/docs/html/exception_handling.html
    
    const PEXCEPTION_RECORD record = info->ExceptionRecord;
    const DWORD code = record->ExceptionCode;   // System code. See STATUS_* macros in winnt.h
    const PVOID address = record->ExceptionAddress;
    const PVOID program_counter = reinterpret_cast<PVOID>(info->ContextRecord->Rip);
    
    std::ostringstream oss;
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
    
    throw fseh_exception(code, oss.str());
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
