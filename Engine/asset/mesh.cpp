
#include "asset/mesh.h"

#include "asset/asset_discovery.h"
#include "asset/factories.h"

namespace engine
{
  asset_type mesh::get_static_asset_type()
  {
    return asset_type::static_mesh;
  }

  mesh* mesh::load(const std::string& mesh_name)
  {
    return asset_discovery::load_mesh(mesh_name);
  }

  void mesh::save(mesh* object)
  {
  }

  mesh* mesh::spawn()
  {
    return object_factory::spawn_mesh();
  }
}