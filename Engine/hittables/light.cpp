
#include <sstream>

#include "hittables/light.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "math/math.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

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