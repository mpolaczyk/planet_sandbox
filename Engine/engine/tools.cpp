#include <memory>
#include <Windows.h>

#include "tools.h"

namespace engine
{
    std::string ftools::to_utf8(const std::wstring& wstr)
    {
        const int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        const std::unique_ptr<char[]> buffer(new char[bufferSize]);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.get(), bufferSize, nullptr, nullptr);
        return std::string(buffer.get());
    }

    std::wstring ftools::to_utf16(const std::string& str)
    {
        const int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        const std::unique_ptr<wchar_t[]> buffer(new wchar_t[bufferSize]);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer.get(), bufferSize);
        return std::wstring(buffer.get());
    }

    void ftools::replace(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = str.find(from);
        while(start_pos != std::string::npos)
        {
            start_pos = str.find(from);
            str.replace(start_pos, from.length(), to);
        }
    }

    bool ftools::contains(const std::string& str, const std::string& pattern)
    {
        return str.find(pattern) != std::string::npos;
    }
}