
#include "asset/factories.h"

namespace engine
{
  material* object_factory::spawn_material(material_type type)
  {
    return new material(type);
  }

  mesh* object_factory::spawn_mesh()
  {
    return new mesh();
  }

  texture* object_factory::spawn_texture()
  {
    return new texture();
  }
}