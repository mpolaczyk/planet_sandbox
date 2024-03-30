#pragma once

#include "core/core.h"

#include "math/vec3.h"

namespace engine
{
  class ENGINE_API fhash
  {
  public:
    static uint32_t combine(uint32_t a, uint32_t c);
    static uint32_t combine(uint32_t a, uint32_t b, uint32_t c, uint32_t d = 0);
    static uint32_t get(uint64_t a);
    static uint32_t get(int64_t a);
    static uint32_t get(float a);
    static uint32_t get(double a);
    static uint32_t get(bool a);
    static uint32_t get(const void* a);
    static uint32_t get(void* a);
    static uint32_t get(const fvec3& a);
    static uint32_t get(const char* a);
  };
}
