
#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "assets/vertex_shader.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(vertex_shader_asset, asset_base, Vertex shader asset)
  OBJECT_DEFINE_SPAWN(vertex_shader_asset)
  OBJECT_DEFINE_VISITOR(vertex_shader_asset)
  
  bool vertex_shader_asset::load(vertex_shader_asset* instance, const std::string& name)
  {
    asset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading vertex shader: {0}", name);

    std::ostringstream oss;
    oss << name << ".json";
    const std::string file_path = io::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open vertex shader asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(deserialize_object(j));

    REG.set_custom_display_name(instance->get_runtime_id(), name);

    //if (!load_img(instance->img_file_name, instance->width, instance->height, instance))
    //{
    //  LOG_ERROR("Failed to load texture file: {0}", instance->img_file_name.c_str());
    //  return false;
    //}
    // FIX compile asset
    return true;
  }
}
