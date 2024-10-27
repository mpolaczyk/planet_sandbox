#include "assets/vertex_shader.h"

#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(avertex_shader, ashader, Vertex shader asset)
  OBJECT_DEFINE_SPAWN(avertex_shader)
}
