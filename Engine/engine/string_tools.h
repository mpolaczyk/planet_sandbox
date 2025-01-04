#pragma once

#include <string>

#include "core/core.h"

namespace engine
{
  struct ENGINE_API fstring_tools
  {
    static std::string to_utf8(const std::wstring& wstr);
    static std::wstring to_utf16(const std::string& str);

    static void replace(std::string& str, const std::string& from, const std::string& to);
    static bool contains(const std::string& str, const std::string& pattern);
    static std::string remove_file_extension(const std::string& filename);

    static std::string append(const char* text, int value);
  };
}
