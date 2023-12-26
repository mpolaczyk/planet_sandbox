#pragma once

#include <chrono>
#include "core/core.h"

namespace std {
  template<typename> class function;
}

namespace engine
{
  struct ENGINE_API timer_instance
  {
    inline void start(const std::string& name);
    inline uint64_t repeat(const std::string& name, uint32_t count, const std::function<void()>& func);
    inline uint64_t once(const std::string& name, const std::function<void()>& func);
    inline uint64_t stop();

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_point, end_point;
    std::string name;
  };

  struct ENGINE_API scope_timer
  {
    explicit scope_timer(const std::string& name);
    ~scope_timer();

    timer_instance state;
  };  

  static timer_instance static_instance;
  static void static_start(const std::string& name)
  {
    static_instance.start(name);
  }
  static uint64_t static_stop()
  {
    return static_instance.stop();
  }
}