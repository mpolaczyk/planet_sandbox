#pragma once

#include "core/core.h"

namespace engine
{
  class ENGINE_API stats
  {
  public:
    static void reset();
    static void inc_ray();
    static void inc_ray_triangle_intersection();
    static void inc_ray_box_intersection();
    static void inc_ray_object_intersection();

    static uint64_t get_ray_count();
    static uint64_t get_ray_triangle_intersection_count();
    static uint64_t get_ray_box_intersection_count();
    static uint64_t get_ray_object_intersection_count();
  };
}