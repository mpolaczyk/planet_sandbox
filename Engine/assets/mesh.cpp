#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "assets/mesh.h"

#include "engine/log.h"
#include "engine/io.h"
#include "persistence/assets_json.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"

namespace engine
{
  OBJECT_DEFINE_BASE(static_mesh_asset)
  OBJECT_DEFINE_NOSAVE(static_mesh_asset)

  static_mesh_asset* static_mesh_asset::load(const std::string& name)
  {
    LOG_DEBUG("Loading mesh: {0}", name.c_str());

    std::ostringstream oss;
    oss << name << ".json";
    std::string file_path = io::get_mesh_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open mesh asset: {0}", file_path.c_str());
      return nullptr;
    }

    static_mesh_asset* obj = static_mesh_asset::spawn();

    nlohmann::json j;
    input_stream >> j;
    mesh_serializer::deserialize(j, obj);

    if (!load_obj(obj->obj_file_name, obj->shape_index, obj->faces))
    {
      LOG_ERROR("Failed to load object file: {0}", obj->obj_file_name.c_str());
      return nullptr;
    }

    get_object_registry()->add<static_mesh_asset>(obj, name); // FIX

    return obj;
  }
}