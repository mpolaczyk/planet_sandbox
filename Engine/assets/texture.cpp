
#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "assets/texture.h"

#include "engine/log.h"
#include "engine/io.h"
#include "persistence/assets_json.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(texture_asset, asset_base)
  OBJECT_DEFINE_SPAWN(texture_asset)
  OBJECT_DEFINE_NOSAVE(texture_asset)
 

  texture_asset* texture_asset::load(const std::string& name)
  {
    LOG_DEBUG("Loading texture: {0}", name.c_str());

    std::ostringstream oss;
    oss << name << ".json";
    std::string file_path = io::get_texture_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open texture asset: {0}", file_path.c_str());
      return nullptr;
    }

    texture_asset* obj = texture_asset::spawn(name);
    if (obj == nullptr)
    {
      return nullptr;
    }

    nlohmann::json j;
    input_stream >> j;
    texture_serializer::deserialize(j, obj);

    if (!load_img(obj->img_file_name, obj->width, obj->height, obj))
    {
      LOG_ERROR("Failed to load texture file: {0}", obj->img_file_name.c_str());
      return nullptr;
    }

    return obj;
  }

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