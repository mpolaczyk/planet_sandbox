
#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "assets/material.h"
#include "engine/log.h"
#include "engine/io.h"
#include "persistence/assets_json.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE(material_asset, asset_base, Material asset)
  OBJECT_DEFINE_SPAWN(material_asset)
  
  bool material_asset::load(material_asset* instance, const std::string& name)
  {
    asset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading material: {0}", name.c_str());

    std::ostringstream oss;
    oss << name << ".json";
    std::string file_path = io::get_material_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open material asset: {0}", file_path.c_str());
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    material_serializer::deserialize(j, instance);

    REG.set_custom_display_name(instance->get_runtime_id(), name);

    return true;
  }

  void material_asset::save(material_asset* object)
  {
    assert(object != nullptr);

    nlohmann::json j;
    j = material_serializer::serialize(object);

    std::ostringstream oss;
    oss << object->get_class()->class_name << ".json";
    std::ofstream o(io::get_material_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    o.close();
  }

}