#pragma once

#include <atomic>

#include "stats.h"

namespace engine
{
  static std::atomic<uint64_t> rays{};
  static std::atomic<uint64_t> ray_triangle_intersection{};
  static std::atomic<uint64_t> ray_box_intersection{};
  static std::atomic<uint64_t> ray_object_intersection{};

  void fstats::reset()
  {
    rays = 0;
    ray_box_intersection = 0;
    ray_triangle_intersection = 0;
    ray_object_intersection = 0;
  }
  void fstats::inc_ray()
  {
#if USE_STAT
    rays++;
#endif
  }
  void fstats::inc_ray_triangle_intersection()
  {
#if USE_STAT
    ray_triangle_intersection++;
#endif
  }
  void fstats::inc_ray_box_intersection()
  {
#if USE_STAT
    ray_box_intersection++;
#endif
  }
  void fstats::inc_ray_object_intersection()
  {
#if USE_STAT
    ray_object_intersection++;
#endif
  }
  uint64_t fstats::get_ray_count()
  {
    return rays;
  }
  uint64_t fstats::get_ray_triangle_intersection_count()
  {
    return ray_triangle_intersection;
  }
  uint64_t fstats::get_ray_box_intersection_count()
  {
    return ray_box_intersection;
  }
  uint64_t fstats::get_ray_object_intersection_count()
  {
    return ray_object_intersection;
  }
}
