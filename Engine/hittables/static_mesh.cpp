
#include <sstream>

#include "hittables/static_mesh.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "engine/hash.h"
#include "profile/stats.h"
#include "math/math.h"
#include "math/vertex_data.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(hstatic_mesh, hhittable_base, Static mesh)
  OBJECT_DEFINE_SPAWN(hstatic_mesh)
  OBJECT_DEFINE_VISITOR(hstatic_mesh)
  
  bool hstatic_mesh::get_bounding_box(faabb& out_box) const
  {
    // TODO rotate mesh asset
    // mesh_asset_ptr.get()->bounding_box
    return true;
  }


  inline uint32_t hstatic_mesh::get_hash() const
  {
    uint32_t a = fhash::combine(hhittable_base::get_hash(), fhash::get(origin), fhash::get(extent), fhash::get(rotation));
    uint32_t b = fhash::combine(fhash::get(scale), fhash::get(material_asset_ptr.get_name().c_str()));
    return fhash::combine(a, b);
  }

  hstatic_mesh* hstatic_mesh::clone() const
  {
    return REG.copy_shallow<hstatic_mesh>(this);
  }

  void hstatic_mesh::load_resources()
  {
    mesh_asset_ptr.get();
  }
}