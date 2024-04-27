#pragma once

#include <chrono>
#include "core/core.h"

namespace std {
  template<typename> class function;
}

namespace engine
{
  struct ENGINE_API ftimer_instance
  {
    inline void start(const std::string& name);
    inline uint64_t repeat(const std::string& name, uint32_t count, const std::function<void()>& func);  // microseconds
    inline uint64_t once(const std::string& name, const std::function<void()>& func);  // microseconds
    inline uint64_t stop(); // microseconds
    inline bool is_working() const { return is_started; }

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_point, end_point;
    std::string name;
    bool is_started = false;
  };

  struct ENGINE_API fscope_timer
  {
    explicit fscope_timer(const std::string& name, uint64_t* out_time);
    ~fscope_timer();

    uint64_t* time_ptr;
    ftimer_instance state;
  };  

  static ftimer_instance static_instance;
  static void static_start(const std::string& name)
  {
    static_instance.start(name);
  }
  static uint64_t static_stop()
  {
    return static_instance.stop();
  }
}