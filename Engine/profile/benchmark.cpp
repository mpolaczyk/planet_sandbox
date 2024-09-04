#include <functional>
#include <chrono>

#include "profile/benchmark.h"

#if USE_PIX
#include "core/windows_minimal.h"
#include "pix3.h" // https://devblogs.microsoft.com/pix/winpixeventruntime
#endif

#include "engine/log.h"

namespace engine
{
  void ftimer_instance::start()
  {
    is_started = true;
#if USE_PIX
    PIXBeginEvent(PIX_COLOR(155, 112, 0), name.c_str());
#endif
#if USE_BENCHMARK
    start_point = std::chrono::high_resolution_clock::now();
#endif
  }  
  void ftimer_instance::start(const std::string& in_name)
  {
#if USE_BENCHMARK
    name = in_name;
#endif
    start();
  }

  uint64_t ftimer_instance::repeat(const std::string& name, uint32_t count, const std::function<void()>& func)
  {
    uint64_t sum = 0;
    for(uint32_t i = 0; i < count; i++)
    {
      start(name);
      func();
      sum += stop();
    }
    return sum;
  }

  uint64_t ftimer_instance::once(const std::string& name, const std::function<void()>& func)
  {
    return repeat(name, 1, func);
  }

  uint64_t ftimer_instance::stop(bool log)
  {
    is_started = false;
#if USE_PIX
    PIXEndEvent();
#endif
#if USE_BENCHMARK
    end_point = std::chrono::high_resolution_clock::now();
    uint64_t begin = time_point_cast<std::chrono::microseconds>(start_point).time_since_epoch().count();
    uint64_t end = time_point_cast<std::chrono::microseconds>(end_point).time_since_epoch().count();
    last_time_us = end - begin;
    if(log)
    {
      LOG_TRACE("Benchmark: {0} {1}[us] = {2}[ms] = {3}[s]", name, last_time_us, last_time_us / 1000, last_time_us / 1000000);
    }
    return last_time_us;
#else
    return 0;
#endif
  }

  fscope_timer::fscope_timer(ftimer_instance& other)
  {
    time_ptr = nullptr;
    state = &other;
    state->start();
  }
  
  fscope_timer::fscope_timer(const std::string& in_name, uint64_t* out_time_us)
  {
    time_ptr = out_time_us;
    state = new ftimer_instance();
    state->start(in_name);
  }

  fscope_timer::~fscope_timer()
  {
    uint64_t time = state->stop();
    if(time_ptr)
    {
      *time_ptr = time;
    }
  }
}
