#include <sstream>

#include "hittables/hittables.h"

#include "math/aabb.h"
#include "math/onb.h"
#include "assets/material.h"
#include "engine/log.h"
#include "profile/stats.h"
#include "engine/hash.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(hittable, object)
  OBJECT_DEFINE_NOSPAWN(hittable)

  std::string hittable::get_name() const
  {
    std::string base_name = "hittable";// FIX object_type_names[(int)type];

    std::ostringstream oss;
    oss << "[" << get_runtime_id() << "]" << "/" << base_name << "/" << material_asset_ptr.get_name();
    return oss.str();
  }

  inline uint32_t hittable::get_hash() const
  {
    return hash::combine(hash::get(material_asset_ptr.get_name().c_str()), hash::get(type));
  }

  void hittable::load_resources()
  {
    // Materials are loaded on editor startup because I need to show all of them in UI
    // This is here for consistence:
    material_asset_ptr.get();
  }

}