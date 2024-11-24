#include <sstream>

#include "hittables/static_mesh.h"

#include "core/application.h"
#include "hittables/hittables.h"
#include "engine/log.h"
#include "engine/hash.h"
#include "math/math.h"
#include "profile/stats.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

#include "reactphysics3d/engine/PhysicsCommon.h"
#include "reactphysics3d/mathematics/Vector3.h"

namespace engine
{
  OBJECT_DEFINE(hstatic_mesh, hhittable_base, Static mesh)
  OBJECT_DEFINE_SPAWN(hstatic_mesh)
  OBJECT_DEFINE_VISITOR(hstatic_mesh)

  inline uint32_t hstatic_mesh::get_hash() const
  {
    return fhash::combine(hhittable_base::get_hash(), fhash::get(material_asset_ptr.get_name().c_str()), fhash::get(mesh_asset_ptr.get_name().c_str()));
  }

  void hstatic_mesh::load_resources()
  {
    hhittable_base::load_resources();
    mesh_asset_ptr.get();
    material_asset_ptr.get();
  }

  void hstatic_mesh::create_physics_state()
  {
    using namespace reactphysics3d;

    hhittable_base::create_physics_state();

    const DirectX::BoundingBox& box = mesh_asset_ptr.get()->bounding_box;
    if(box.Extents.x != 0.0f && box.Extents.y != 0.0f && box.Extents.z != 0.0f)
    {
      const reactphysics3d::Vector3 positive_half_extents(fabs(box.Extents.x * scale.x), fabs(box.Extents.y * scale.y), fabs(box.Extents.z * scale.z));
      box_shape = fapplication::get_instance()->physics_common->createBoxShape(positive_half_extents);      
      collider = rigid_body->addCollider(box_shape, Transform::identity());
    }
    else
    {
      LOG_WARN("Unable to create collider for object: {0} with mesh {1}", get_display_name(), mesh_asset_ptr.get()->name)
    }
  }

  void hstatic_mesh::destroy_physics_state()
  {
    if(collider)
    {
      rigid_body->removeCollider(collider);
      collider = nullptr;
    }
    if(box_shape)
    {
      fapplication::get_instance()->physics_common->destroyBoxShape(box_shape);
      box_shape = nullptr;
    }

    hhittable_base::destroy_physics_state();
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
