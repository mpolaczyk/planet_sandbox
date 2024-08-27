#include "onb.h"

#include <corecrt_math.h>
#include "math/math.h"

namespace engine
{
  fvec3 fonb::local(const fvec3& a) const
  {
    return a.x * u + a.y * v + a.z * w;
  }

  void fonb::build_from_w(const fvec3& in_w)
  {
    w = fmath::normalize(in_w);
    fvec3 a = (fabs(w.x) > 0.9f) ? fvec3(0.0f, 1.0f, 0.0f) : fvec3(1.0f, 0.0f, 0.0f);
    v = fmath::normalize(fmath::cross(w, a));
    u = fmath::cross(w, v);
  }
}
