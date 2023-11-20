
#include <sstream>

#include "assets/texture.h"

#include "asset/asset_io.h"

namespace engine
{
  OBJECT_DEFINE_BASE(texture_asset)
  OBJECT_DEFINE_NOSAVE(texture_asset)
  OBJECT_DEFINE_LOAD(texture_asset)

  std::string texture_asset::get_display_name() const
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