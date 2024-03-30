
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
  void ftimer_instance::start(const std::string& in_name)
  {
#if USE_PIX
    PIXBeginEvent(PIX_COLOR(155, 112, 0), in_name.c_str());
#endif
#if USE_BENCHMARK
    name = in_name;
    start_point = std::chrono::high_resolution_clock::now();
#endif
  }

  uint64_t ftimer_instance::repeat(const std::string& name, uint32_t count, const std::function<void()>& func)
  {
    uint64_t sum = 0;
    for (uint32_t i = 0; i < count; i++)
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

  uint64_t ftimer_instance::stop()
  {
#if USE_PIX
    PIXEndEvent();
#endif
#if USE_BENCHMARK
    end_point = std::chrono::high_resolution_clock::now();
    uint64_t begin = time_point_cast<std::chrono::microseconds>(start_point).time_since_epoch().count();
    uint64_t end = time_point_cast<std::chrono::microseconds>(end_point).time_since_epoch().count();
    uint64_t time = end - begin;

    LOG_TRACE("Benchmark: {0} {1}[us] = {2}[ms] = {3}[s]", name, time, time / 1000, time / 1000000);
    
    return time;
#else
    return 0;
#endif
  }
  
  fscope_timer::fscope_timer(const std::string& name)
  {
    state.start(name);
  }

  fscope_timer::~fscope_timer()
  {
    state.stop();
  }
}
