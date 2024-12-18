#include <string>

#include "engine/math/hash.h"

#include "engine/math/vec3.h"
#include <DirectXMath.h>

namespace engine
{
  uint32_t fhash::combine(uint32_t a, uint32_t b)
  {
    uint32_t c = 0x9e3779b9;
    a += c;

    a -= c;
    a -= b;
    a ^= (b >> 13);
    c -= b;
    c -= a;
    c ^= (a << 8);
    b -= a;
    b -= c;
    b ^= (c >> 13);
    a -= c;
    a -= b;
    a ^= (b >> 12);
    c -= b;
    c -= a;
    c ^= (a << 16);
    b -= a;
    b -= c;
    b ^= (c >> 5);
    a -= c;
    a -= b;
    a ^= (b >> 3);
    c -= b;
    c -= a;
    c ^= (a << 10);
    b -= a;
    b -= c;
    b ^= (c >> 15);

    return b;
  }

  uint32_t fhash::combine(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
  {
    return combine(combine(a, b), combine(c, d));
  }

  uint32_t fhash::get(uint64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }

  uint32_t fhash::get(int64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }

  uint32_t fhash::get(float a)
  {
    return reinterpret_cast<uint32_t&>(a);
  }

  uint32_t fhash::get(double a)
  {
    return get(reinterpret_cast<uint64_t&>(a));
  }

  uint32_t fhash::get(void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }

  uint32_t fhash::get(const void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }

  uint32_t fhash::get(bool a)
  {
    return (uint32_t)a;
  }

  uint32_t fhash::get(const fvec3& a)
  {
    return combine(get(a.x), get(a.y), get(a.z), get(a.padding));
  }

  uint32_t fhash::get(const DirectX::XMFLOAT3& a)
  {
    return combine(get(a.x), get(a.y), get(a.z), 1);
  }

  uint32_t fhash::get(const DirectX::XMFLOAT4& a)
  {
    return combine(get(a.x), get(a.y), get(a.z), get(a.w));
  }
  
  uint32_t fhash::get(const DirectX::XMFLOAT4X4& a)
  {
    uint32_t x = combine(get(a.m[0][0]), get(a.m[1][0]), get(a.m[2][0]), get(a.m[3][0]));
    uint32_t y = combine(get(a.m[0][1]), get(a.m[1][1]), get(a.m[2][1]), get(a.m[3][1]));
    uint32_t z = combine(get(a.m[0][2]), get(a.m[1][2]), get(a.m[2][2]), get(a.m[3][2]));
    uint32_t w = combine(get(a.m[0][3]), get(a.m[1][3]), get(a.m[2][3]), get(a.m[3][3]));
    return combine(x,y,z,w);
  }
  
  uint32_t fhash::get(const char* a)
  {
    return static_cast<uint32_t>(std::hash<std::string>{}(a));
  }
}
