#include "stdafx.h"

#include "assets/texture.h"

#include "engine/io.h"
#include "engine/resources/resources_io.h"
#include "engine/persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(atexture, aasset_base, Texture asset)
  OBJECT_DEFINE_SPAWN(atexture)
  OBJECT_DEFINE_VISITOR(atexture)

  std::string atexture::get_extension() const
  {
    return fio::get_texture_extension().c_str();
  }
  
  std::string atexture::get_folder() const
  {
    return fio::get_textures_dir().c_str();
  }
  
  bool atexture::load(const std::string& name)
  {
    aasset_base::load(name);

    LOG_DEBUG("Loading texture: {0}", name);

    std::ostringstream oss;
    oss << name << fio::get_texture_extension();
    const std::string file_path = fio::get_texture_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to open texture: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    accept(vdeserialize_object(j));
    set_display_name(name);

    if(!load_img(img_file_name, this))
    {
      LOG_ERROR("Failed to load texture: {0}", img_file_name);
      return false;
    }
    return true;
  }
}
