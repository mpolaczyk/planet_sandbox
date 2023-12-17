#include <sstream>

#include "hittables/hittables.h"

#include "math/aabb.h"
#include "math/onb.h"
#include "assets/material.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hittable, object, Hittable)
  OBJECT_DEFINE_NOSPAWN(hittable)
  OBJECT_DEFINE_VISITOR(hittable)
  
  inline uint32_t hittable::get_hash() const
  {
    return hash::combine(hash::get(material_asset_ptr.get_name().c_str()), 0);
  }

  void hittable::load_resources()
  {
    material_asset_ptr.get();
  }

}