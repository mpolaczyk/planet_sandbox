#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "assets/mesh.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(static_mesh_asset, asset_base, Static mesh asset)
  OBJECT_DEFINE_SPAWN(static_mesh_asset)
  OBJECT_DEFINE_VISITOR(static_mesh_asset)

  bool static_mesh_asset::load(static_mesh_asset* instance, const std::string& name)
  {
    asset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading mesh: {0}", name);

    std::ostringstream oss;
    oss << name << ".json";
    const std::string file_path = io::get_mesh_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open mesh asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(deserialize_object(j));

    REG.set_custom_display_name(instance->get_runtime_id(), name);

    if (!load_obj(instance->obj_file_name, instance->shape_index, instance->faces))
    {
      LOG_ERROR("Failed to load object file: {0}", instance->obj_file_name);
      return false;
    }
    return true;
  }
}
