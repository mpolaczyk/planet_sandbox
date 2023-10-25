#include "onb.h"

#include <corecrt_math.h>
#include "math.h"

namespace engine
{
  vec3 onb::local(const vec3& a) const
  {
    return a.x * u + a.y * v + a.z * w;
  }

  void onb::build_from_w(const vec3& in_w)
  {
    w = math::normalize(in_w);
    vec3 a = (fabs(w.x) > 0.9f) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    v = math::normalize(math::cross(w, a));
    u = math::cross(w, v);
  }
}