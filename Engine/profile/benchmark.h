#pragma once

#include <chrono>
#include "core/core.h"

namespace std {
  template<typename> class function;
}

namespace engine
{
  struct ENGINE_API instance
  {
    inline void start(const std::string& name);
    inline uint64_t repeat(const std::string& name, uint32_t count, const std::function<void()>& func);
    inline uint64_t once(const std::string& name, const std::function<void()>& func);
    inline uint64_t stop();

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_point, end_point;
    std::string name;
  };

  struct ENGINE_API scope_counter
  {
    explicit scope_counter(const std::string& name);
    ~scope_counter();

    instance state;
  };  

  static instance static_instance;
  static void static_start(const std::string& name)
  {
    static_instance.start(name);
  }
  static uint64_t static_stop()
  {
    return static_instance.stop();
  }
}