#include "stdafx.h"

#include "math/textures.h"

#include "app/factories.h"
#include "app/asset_discovery.h"

asset_type texture::get_static_asset_type()
{
  return asset_type::texture;
}

texture* texture::load(const std::string& texture_name)
{
  return asset_discovery::load_texture(texture_name);
}

texture* texture::spawn()
{
  return object_factory::spawn_texture();
}