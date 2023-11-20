
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
  OBJECT_DEFINE(material_asset, asset_base)
  OBJECT_DEFINE_SPAWN(material_asset)

  material_asset* material_asset::load(const std::string& name)
  {
    LOG_DEBUG("Loading material: {0}", name.c_str());

    std::ostringstream oss;
    oss << name << ".json";
    std::string file_path = io::get_material_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open material asset: {0}", file_path.c_str());
      return nullptr;
    }

    material_asset* obj = material_asset::spawn();

    nlohmann::json j;
    input_stream >> j;
    material_serializer::deserialize(j, obj);

    get_object_registry()->add<material_asset>(obj, name);    // FIX

    return obj;
  }

  void material_asset::save(material_asset* object)
  {
    assert(object != nullptr);

    nlohmann::json j;
    j = material_serializer::serialize(object);

    std::ostringstream oss;
    oss << object->get_name() << ".json";
    std::ofstream o(io::get_material_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    o.close();
  }

  std::string material_asset::get_display_name() const
  {
    std::ostringstream oss;
    oss << object::get_display_name() << " - " << material_type_names[(int)type];
    return oss.str();
  }
}