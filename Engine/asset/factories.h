#pragma once

// No RTTI, simple type detection for each object type

#include "asset/materials.h"
#include "asset/mesh.h"
#include "asset/textures.h"

namespace engine
{
  class ENGINE_API object_factory
  {
  public:

    static material* spawn_material(material_type type);

    static mesh* spawn_mesh();

    static texture* spawn_texture();
  };
}