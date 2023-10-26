
#include <sstream>

#include "asset/textures.h"

namespace engine
{
  asset_type texture::get_static_asset_type()
  {
    return asset_type::texture;
  }

  texture* texture::load(const std::string& texture_name)
  {
    return nullptr;
    //return asset_discovery::load_texture(texture_name); FIX
  }

  void texture::save(texture* object)
  {
  }

  texture* texture::spawn()
  {
    //return object_factory::spawn_texture();  FIX
    return nullptr;
  }

  std::string texture::get_display_name() const
  {
    std::ostringstream oss;
    std::string quality = "LDR";
    if (is_hdr)
    {
      quality = "HDR";
    }
    oss << asset::get_display_name() << " " << width << "x" << height << " " << quality;
    return oss.str();
  }
}