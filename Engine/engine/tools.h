#pragma once

#include <string>

#include "core/core.h"

namespace engine
{
    struct ENGINE_API ftools
    {
        static std::string to_utf8(const std::wstring& wstr);
        static std::wstring to_utf16(const std::string& str);
    };
}

