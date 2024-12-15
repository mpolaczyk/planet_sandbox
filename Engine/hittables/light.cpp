#include <sstream>

#include "hittables/light.h"
#include "engine/hittable.h"
#include "engine/log.h"
#include "engine/math/hash.h"
#include "engine/math/math.h"
#include "core/rtti/object_registry.h"
#include "core/rtti/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hlight, hhittable_base, Light)
  OBJECT_DEFINE_SPAWN(hlight)
  OBJECT_DEFINE_VISITOR(hlight)

  inline uint32_t hlight::get_hash() const
  {
    return fhash::combine(hhittable_base::get_hash(), fhash::get(properties.color));
  }
}
