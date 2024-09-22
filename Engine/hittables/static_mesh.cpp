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

  void hstatic_mesh::get_object_matrices(const XMFLOAT4X4& view_projection, fobject_data& out_data)
  {
    const XMMATRIX translation_matrix = XMMatrixTranslation(origin.x, origin.y, origin.z);
    const XMMATRIX rotation_matrix =
        XMMatrixRotationX(XMConvertToRadians(rotation.x))
      * XMMatrixRotationY(XMConvertToRadians(rotation.y))
      * XMMatrixRotationZ(XMConvertToRadians(rotation.z));
    const XMMATRIX scale_matrix = XMMatrixScaling(scale.x, scale.y, scale.z);
    const XMMATRIX world_matrix = scale_matrix * rotation_matrix * translation_matrix;
    const XMMATRIX inverse_transpose_model_world = XMMatrixTranspose(XMMatrixInverse(nullptr, world_matrix));
    const XMMATRIX model_world_view_projection = XMMatrixMultiply(world_matrix, XMLoadFloat4x4(&view_projection));

    XMStoreFloat4x4(&out_data.model_world, world_matrix);
    XMStoreFloat4x4(&out_data.inverse_transpose_model_world, inverse_transpose_model_world);
    XMStoreFloat4x4(&out_data.model_world_view_projection, model_world_view_projection);
  }
}
