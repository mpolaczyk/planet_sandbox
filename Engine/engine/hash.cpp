
#include <functional>

#include "hash.h"

namespace engine
{
  uint32_t hash::combine(uint32_t a, uint32_t b)
  {
    uint32_t c = 0x9e3779b9;
    a += c;

    a -= c; a -= b; a ^= (b >> 13);
    c -= b; c -= a; c ^= (a << 8);
    b -= a; b -= c; b ^= (c >> 13);
    a -= c; a -= b; a ^= (b >> 12);
    c -= b; c -= a; c ^= (a << 16);
    b -= a; b -= c; b ^= (c >> 5);
    a -= c; a -= b; a ^= (b >> 3);
    c -= b; c -= a; c ^= (a << 10);
    b -= a; b -= c; b ^= (c >> 15);

    return b;
  }

  uint32_t hash::combine(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
  {
    return combine(combine(a, b), combine(c, d));
  }

  uint32_t hash::get(uint64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }

  uint32_t hash::get(int64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }

  uint32_t hash::get(float a)
  {
    return reinterpret_cast<uint32_t&>(a);
  }

  uint32_t hash::get(double a)
  {
    return get(reinterpret_cast<uint64_t&>(a));
  }

  uint32_t hash::get(const void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }

  uint32_t hash::get(void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }

  uint32_t hash::get(bool a)
  {
    return (uint32_t)a;
  }

  uint32_t hash::get(const vec3& a)
  {
    return combine(get(a.x), get(a.y), get(a.z), get(a.padding));
  }

  uint32_t hash::get(const std::string& a)
  {
    return static_cast<uint32_t>(std::hash<std::string>{}(a));
  }
}