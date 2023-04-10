#pragma once

#include <chrono>
#include <functional>

using namespace std::chrono;

namespace benchmark
{
  struct instance
  {
    inline void start(const char* name, bool verbose = true);
    inline uint64_t repeat(const char* name, uint32_t count, std::function<void()>& func, bool verbose = true);
    inline uint64_t once(const char* name, std::function<void()>& func, bool verbose = true);
    inline uint64_t stop();

  private:
    time_point<high_resolution_clock> start_point, end_point;
    const char* name;
    bool verbose;
  };

  struct scope_counter
  {
    scope_counter(const char* name, bool verbose = true);
    ~scope_counter();

    instance state;
  };  

  static instance static_instance;
  static void static_start(const char* name, bool verbose = true)
  {
    static_instance.start(name, verbose);
  }
  static uint64_t static_stop()
  {
    return static_instance.stop();
  }
}