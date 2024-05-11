
#include <sstream>

#include "hittables/static_mesh.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "engine/hash.h"
#include "profile/stats.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hstatic_mesh, hhittable_base, Static mesh)
  OBJECT_DEFINE_SPAWN(hstatic_mesh)
  OBJECT_DEFINE_VISITOR(hstatic_mesh)

  inline uint32_t hstatic_mesh::get_hash() const
  {
    return fhash::combine(hhittable_base::get_hash(), fhash::get(material_asset_ptr.get_name().c_str()), fhash::get(mesh_asset_ptr.get_name().c_str()), fhash::combine(fhash::get(bounding_box.maximum), fhash::get(bounding_box.minimum)));
  }

  void hstatic_mesh::load_resources()
  {
    hhittable_base::load_resources();
    mesh_asset_ptr.get();
    material_asset_ptr.get();
  }
}