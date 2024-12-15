#include "assets/vertex_shader.h"

#include "engine/io.h"
#include "core/rtti/object_registry.h"
#include "core/rtti/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(avertex_shader, ashader, Vertex shader asset)
  OBJECT_DEFINE_SPAWN(avertex_shader)

    std::string avertex_shader::get_extension() const
  {
    return fio::get_vertex_shader_extension().c_str();
  }
}
