#include "assets/pixel_shader.h"

#include "engine/io.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"

namespace engine
{
  OBJECT_DEFINE(apixel_shader, ashader, Pixel shader asset)
  OBJECT_DEFINE_SPAWN(apixel_shader)

  std::string apixel_shader::get_extension() const
  {
    return fio::get_pixel_shader_extension().c_str();
  }
 }
