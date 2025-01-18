#include "stdafx.h"

#include "assets/pixel_shader.h"

#include "engine/io.h"

namespace engine
{
  OBJECT_DEFINE(apixel_shader, ashader, Pixel shader asset)
  OBJECT_DEFINE_SPAWN(apixel_shader)

  std::string apixel_shader::get_extension() const
  {
    return fio::get_pixel_shader_extension();
  }
 }
