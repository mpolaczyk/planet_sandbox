
#include <sstream>

#include "asset/textures.h"

#include "asset/asset_discovery.h"

namespace engine
{
  object_type texture::get_static_type()
  {
    return object_type::texture;
  }

  texture* texture::load(const std::string& texture_name)
  {
    return asset_discovery::load_texture(texture_name);
  }

  void texture::save(texture* object)
  {
  }

  texture* texture::spawn()
  {
    return new texture();
  }

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