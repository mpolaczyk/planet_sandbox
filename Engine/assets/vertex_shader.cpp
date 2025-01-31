#include "stdafx.h"

#include "assets/vertex_shader.h"

#include "engine/io.h"

namespace engine
{
  OBJECT_DEFINE(avertex_shader, ashader, Vertex shader asset)
  OBJECT_DEFINE_SPAWN(avertex_shader)

    std::string avertex_shader::get_extension() const
  {
    return fio::get_vertex_shader_extension();
  }
}
