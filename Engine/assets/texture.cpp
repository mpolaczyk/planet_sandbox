
#include <sstream>

#include "assets/texture.h"

#include "asset/asset_io.h"

namespace engine
{
  OBJECT_DEFINE_BASE(texture)
  OBJECT_DEFINE_NOSAVE(texture)
  OBJECT_DEFINE_LOAD(texture)

  std::string texture::get_display_name() const
  {
    std::ostringstream oss;
    std::string quality = "LDR";
    if (is_hdr)
    {
      quality = "HDR";
    }
    oss << object::get_display_name() << " " << width << "x" << height << " " << quality;
    return oss.str();
  }
}