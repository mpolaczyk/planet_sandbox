
#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "assets/material.h"
#include "engine/log.h"
#include "engine/io.h"
#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(amaterial, aasset_base, Material asset)
  OBJECT_DEFINE_SPAWN(amaterial)
  OBJECT_DEFINE_VISITOR(amaterial)
  
  bool amaterial::load(amaterial* instance, const std::string& name)
  {
    aasset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading material: {0}", name);

    std::ostringstream oss;
    oss << name << ".json";
    const std::string file_path = fio::get_material_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open material asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(vdeserialize_object(j));
    instance->set_display_name(name);

    return true;
  }

  void amaterial::save(amaterial* object)
  {
    assert(object != nullptr);

    nlohmann::json j;
    object->accept(vserialize_object(j));

    std::ostringstream oss;
    oss << object->file_name << ".json";
    std::ofstream o(fio::get_material_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    else
    {
      LOG_ERROR("Unable to save file {0}", oss.str());
    }
    o.close();
  }

}
