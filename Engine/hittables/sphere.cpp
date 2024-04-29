
#include <sstream>

#include "hittables/sphere.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "math/math.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hsphere, hhittable_base, Sphere)
  OBJECT_DEFINE_SPAWN(hsphere)
  OBJECT_DEFINE_VISITOR(hsphere)

  inline uint32_t hsphere::get_hash() const
  {
    return fhash::combine(hhittable_base::get_hash(), fhash::get(radius));
  }
}