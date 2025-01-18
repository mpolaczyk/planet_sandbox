#include "stdafx.h"

#include "assets/ray_tracing_shader.h"

#include "engine/io.h"

namespace engine
{
  OBJECT_DEFINE(aray_tracing_shader, ashader, Ray tracing shader asset)
  OBJECT_DEFINE_SPAWN(aray_tracing_shader)

  std::string aray_tracing_shader::get_extension() const
  {
    return fio::get_ray_tracing_shader_extension();
  }
}
