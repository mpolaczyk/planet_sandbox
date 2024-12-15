#pragma once

#include <chrono>
#include "core/core.h"

namespace std
{
  template <typename>
  class function;
}

namespace engine
{
  struct ENGINE_API ftimer_instance
  {
    inline void start();
    inline void start(const std::string& name);
    inline uint64_t repeat(const std::string& name, uint32_t count, const std::function<void()>& func); // microseconds
    inline uint64_t once(const std::string& name, const std::function<void()>& func); // microseconds
    inline uint64_t stop(bool log = false); // microseconds
    inline bool is_working() const { return is_started; }
    inline uint64_t get_last_time_us() { return last_time_us; }
    inline float get_last_time_ms() { return static_cast<float>(last_time_us) / 1000.0f; }

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_point, end_point;
    uint64_t last_time_us = 0;
    std::string name;
    bool is_started = false;
  };

  struct ENGINE_API fscope_timer
  {
    explicit fscope_timer(ftimer_instance& other);
    explicit fscope_timer(const std::string& name, uint64_t* out_time_us);
    ~fscope_timer();

    uint64_t* time_ptr;
    ftimer_instance* state;
  };
}
