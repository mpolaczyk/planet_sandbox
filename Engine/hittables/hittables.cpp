#include <sstream>

#include "hittables/hittables.h"

#include "math/onb.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hhittable_base, oobject, Hittable)
  OBJECT_DEFINE_NOSPAWN(hhittable_base)
  OBJECT_DEFINE_VISITOR(hhittable_base)
  
  inline uint32_t hhittable_base::get_hash() const
  {
    return fhash::combine(fhash::get(origin), fhash::get(rotation), fhash::get(scale));
  }

}