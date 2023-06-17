#include "stdafx.h"

#include <fstream>

#include "app/asset_discovery.h"
#include "app/factories.h"

#include "json/assets_json.h"

#include "math/materials.h"
#include "math/mesh.h"

material* asset_discovery::load_material(const std::string& material_name)
{
  logger::debug("Loading material: {0}", material_name.c_str());

  std::ostringstream oss;
  oss << material_name << ".json";
  std::string file_path = io::get_material_file_path(oss.str().c_str());
  std::ifstream input_stream(file_path.c_str());
  if (input_stream.fail())
  {
    logger::error("Unable to open material asset: {0}", file_path.c_str());
    return nullptr;
  }

  nlohmann::json j;
  input_stream >> j;
  material_type type = material_type::none;
  if (!TRY_PARSE(material_type, j, "type", type))
  {
    return nullptr;
  }

  material* obj = object_factory::spawn_material(type);
  if (obj == nullptr)
  {
    logger::error("Invalid material type: {0}", static_cast<int>(type));
  }

  material_serializer::deserialize(j, obj);

  return obj;
}

mesh* asset_discovery::load_mesh(const std::string& mesh_name)
{
  logger::debug("Loading mesh: {0}", mesh_name.c_str());

  std::ostringstream oss;
  oss << mesh_name << ".json";
  std::string file_path = io::get_mesh_file_path(oss.str().c_str());
  std::ifstream input_stream(file_path.c_str());
  if (input_stream.fail())
  {
    logger::error("Unable to open mesh asset: {0}", file_path.c_str());
    return nullptr;
  }

  mesh* obj = object_factory::spawn_mesh();
  if (obj == nullptr)
  {
    logger::error("Failed to spawn mesh object.");
  }

  nlohmann::json j;
  input_stream >> j;
  mesh_serializer::deserialize(j, obj);

  if (!obj_helper::load_obj(obj->obj_file_name, obj->shape_index, obj->faces))
  {
    logger::error("Failed to load object file: {0}", obj->obj_file_name.c_str());
  }

  return obj;
}