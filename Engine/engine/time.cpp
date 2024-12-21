
#include <chrono>
#include <filesystem>

#include "engine/time.h"

namespace engine
{
  using namespace std::chrono;

  uint64_t ftime::get_now_us()
  {
    return time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count();
  }
  
  uint64_t ftime::get_now_ms()
  {
    return time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();
  }

  uint64_t ftime::get_file_write_time(const char* file_path)
  {
    using namespace std::filesystem;
    return time_point_cast<milliseconds>(last_write_time(file_path)).time_since_epoch().count();
  }
}