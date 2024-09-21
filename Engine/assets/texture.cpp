#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "assets/texture.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(atexture, aasset_base, Texture asset)
  OBJECT_DEFINE_SPAWN(atexture)
  OBJECT_DEFINE_VISITOR(atexture)

  bool atexture::load(atexture* instance, const std::string& name)
  {
    aasset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading texture: {0}", name);

    std::ostringstream oss;
    oss << name << ".texture";
    const std::string file_path = fio::get_texture_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to open texture asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(vdeserialize_object(j));
    instance->set_display_name(name);

    if(!load_img(instance->img_file_name, instance))
    {
      LOG_ERROR("Failed to load texture file: {0}", instance->img_file_name);
      return false;
    }
    return true;
  }
}
