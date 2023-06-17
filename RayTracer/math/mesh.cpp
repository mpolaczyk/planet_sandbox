#include "stdafx.h"

#include "app/asset_discovery.h"
#include "app/factories.h"

#include "math/mesh.h"

asset_type mesh::get_static_asset_type()
{
  return asset_type::static_mesh;
}

mesh* mesh::load(const std::string& mesh_name)
{
  return asset_discovery::load_mesh(mesh_name);
}

mesh* mesh::spawn()
{
  return object_factory::spawn_mesh();
}
