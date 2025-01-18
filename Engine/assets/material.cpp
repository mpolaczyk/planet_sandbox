#include "stdafx.h"

#include "assets/material.h"
#include "engine/io.h"
#include "engine/persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(amaterial, aasset_base, Material asset)
  OBJECT_DEFINE_SPAWN(amaterial)
  OBJECT_DEFINE_VISITOR(amaterial)

  std::string amaterial::get_extension() const
  {
    return fio::get_material_extension();
  }
  
  std::string amaterial::get_folder() const
  {
    return fio::get_materials_dir();
  }
  
  bool amaterial::load(const std::string& name)
  {
    aasset_base::load(name);

    LOG_DEBUG("Loading material: {0}", name);

    std::ostringstream oss;
    oss << name << fio::get_material_extension();
    const std::string file_path = fio::get_material_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to open material: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    accept(vdeserialize_object(j));
    set_display_name(name);

    return true;
  }

  void amaterial::save()
  {
    nlohmann::json j;
    accept(vserialize_object(j));

    std::ostringstream oss;
    oss << name << fio::get_material_extension();
    std::ofstream o(fio::get_material_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if(o.is_open())
    {
      o.write(str.data(), str.length());
    }
    else
    {
      LOG_ERROR("Unable to save material: {0}", oss.str());
    }
    o.close();
  }
}
