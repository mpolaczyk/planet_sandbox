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
  OBJECT_DEFINE(astatic_mesh, aasset_base, Static mesh asset)
  OBJECT_DEFINE_SPAWN(astatic_mesh)
  OBJECT_DEFINE_VISITOR(astatic_mesh)

  bool astatic_mesh::load(astatic_mesh* instance, const std::string& name)
  {
    aasset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading mesh: {0}", name);

    std::ostringstream oss;
    oss << name << ".json";
    const std::string file_path = fio::get_mesh_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open mesh asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(vdeserialize_object(j));
    instance->set_display_name(name);
    
    if (!load_obj(instance->obj_file_name, instance))
    {
      LOG_ERROR("Failed to load object file: {0}", instance->obj_file_name);
      return false;
    }
    return true;
  }
}
