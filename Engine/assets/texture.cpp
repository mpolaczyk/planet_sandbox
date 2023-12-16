
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
  OBJECT_DEFINE(texture_asset, asset_base, Texture asset)
  OBJECT_DEFINE_SPAWN(texture_asset)
 

  bool texture_asset::load(texture_asset* instance, const std::string& name)
  {
    asset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading texture: {0}", name.c_str());

    std::ostringstream oss;
    oss << name << ".json";
    std::string file_path = io::get_texture_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open texture asset: {0}", file_path.c_str());
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    texture_serializer::deserialize(j, instance);

    REG.set_custom_display_name(instance->get_runtime_id(), name);

    if (!load_img(instance->img_file_name, instance->width, instance->height, instance))
    {
      LOG_ERROR("Failed to load texture file: {0}", instance->img_file_name.c_str());
      return false;
    }
    return true;
  }
}