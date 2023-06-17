#include "stdafx.h"

#include <fstream>

#include "gfx/stb_image.h"

#include "app/asset_discovery.h"
#include "app/factories.h"

#include "json/assets_json.h"

#include "math/materials.h"
#include "math/mesh.h"
#include "math/textures.h"

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

texture* asset_discovery::load_texture(const std::string& texture_name)
{
  logger::debug("Loading texture: {0}", texture_name.c_str());

  std::ostringstream oss;
  oss << texture_name << ".json";
  std::string file_path = io::get_texture_file_path(oss.str().c_str());
  std::ifstream input_stream(file_path.c_str());
  if (input_stream.fail())
  {
    logger::error("Unable to open texture asset: {0}", file_path.c_str());
    return nullptr;
  }

  texture* obj = object_factory::spawn_texture();
  if (obj == nullptr)
  {
    logger::error("Failed to spawn texture object.");
  }

  nlohmann::json j;
  input_stream >> j;
  texture_serializer::deserialize(j, obj);

  
  obj->is_hdr = static_cast<bool>(stbi_is_hdr(file_path.c_str()));

  if (obj->is_hdr)
  {
    obj->data_hdr = stbi_loadf(file_path.c_str(), &obj->width, &obj->height, nullptr, STBI_rgb);
  }
  else
  {
    obj->data_ldr = stbi_load(file_path.c_str(), &obj->width, &obj->height, nullptr, STBI_rgb);
  }

  return obj;
}

void asset_discovery::save_material(const material* object)
{
  assert(object != nullptr);

  nlohmann::json j;
  j = material_serializer::serialize(object);

  std::ostringstream oss;
  oss << object->get_asset_name() << ".json";
  std::ofstream o(io::get_material_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}