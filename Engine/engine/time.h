#pragma once

#include <cstdint>

namespace engine
{
  struct ftime final
  {
    static uint64_t get_now_us();
    static uint64_t get_now_ms();

    static uint64_t get_file_write_time(const char* file_path);
  };
}