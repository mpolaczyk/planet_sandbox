
#include "asset/mesh.h"

#include "asset/asset_discovery.h"

namespace engine
{
  object_type mesh::get_static_type()
  {
    return object_type::static_mesh;
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
    return new mesh();
  }
}