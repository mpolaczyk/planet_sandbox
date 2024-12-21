#include "stdafx.h"

#include "hittables/static_mesh.h"

#include "engine/hittable.h"
#include "engine/math/hash.h"
#include "engine/physics.h"
#include "engine/math/math.h"
#include "assets/mesh.h"
#include "assets/material.h"

#include "reactphysics3d/collision/shapes/BoxShape.h"

namespace engine
{
  OBJECT_DEFINE(hstatic_mesh, hhittable_base, Static mesh)
  OBJECT_DEFINE_SPAWN(hstatic_mesh)
  OBJECT_DEFINE_VISITOR(hstatic_mesh)

  hstatic_mesh::~hstatic_mesh()
  {
    destroy_physics_state();
  }

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
    hhittable_base::create_physics_state();

    const fbounding_box& box = mesh_asset_ptr.get()->bounding_box;
    box_shape = fphysics::create_box_shape_collider(box, scale);
    if(box_shape)
    {
      collider = fphysics::attach_collider(rigid_body, box_shape, box);
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
      fphysics::detach_collider(rigid_body, collider);
      collider = nullptr;
    }
    if(box_shape)
    {
      fphysics::destroy_box_shape_collider(box_shape);
      box_shape = nullptr;
    }

    hhittable_base::destroy_physics_state();
  }

  void hstatic_mesh::transform(const fvec3& in_origin, const fvec3& in_rotation, const fvec3& in_scale)
  {
    if(scale != in_scale)
    {
      const fbounding_box& box = mesh_asset_ptr.get()->bounding_box;
      fphysics::edit_box_shape_collider(box_shape, box, in_scale);
    }
    
    hhittable_base::transform(in_origin, in_rotation, in_scale);
  }

  void hstatic_mesh::get_object_matrices(const XMFLOAT4X4& view_projection, fobject_data& out_data) const
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
