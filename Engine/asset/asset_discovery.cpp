#include <fstream>

#include "asset/asset_discovery.h"

#include "engine/log.h"
#include "engine/io.h"
#include "asset/factories.h"
#include "asset/asset_tools.h"

#include "persistence/assets_json.h"


namespace engine
{
  material* asset_discovery::load_material(const std::string& material_name)
  {
    LOG_DEBUG("Loading material: {0}", material_name.c_str());

    std::ostringstream oss;
    oss << material_name << ".json";
    std::string file_path = io::get_material_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open material asset: {0}", file_path.c_str());
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
      LOG_ERROR("Invalid material type: {0}", static_cast<int>(type));
    }

    material_serializer::deserialize(j, obj);

    return obj;
  }

  mesh* asset_discovery::load_mesh(const std::string& mesh_name)
  {
    LOG_DEBUG("Loading mesh: {0}", mesh_name.c_str());

    std::ostringstream oss;
    oss << mesh_name << ".json";
    std::string file_path = io::get_mesh_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open mesh asset: {0}", file_path.c_str());
      return nullptr;
    }

    mesh* obj = object_factory::spawn_mesh();
    if (obj == nullptr)
    {
      LOG_ERROR("Failed to spawn mesh object.");
      return nullptr;
    }

    nlohmann::json j;
    input_stream >> j;
    mesh_serializer::deserialize(j, obj);

    if (!load_obj(obj->obj_file_name, obj->shape_index, obj->faces))
    {
      LOG_ERROR("Failed to load object file: {0}", obj->obj_file_name.c_str());
      return nullptr;
    }

    return obj;
  }

  texture* asset_discovery::load_texture(const std::string& texture_name)
  {
    LOG_DEBUG("Loading texture: {0}", texture_name.c_str());

    std::ostringstream oss;
    oss << texture_name << ".json";
    std::string file_path = io::get_texture_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open texture asset: {0}", file_path.c_str());
      return nullptr;
    }

    texture* obj = object_factory::spawn_texture();
    if (obj == nullptr)
    {
      LOG_ERROR("Failed to spawn texture object.");
      return nullptr;
    }

    nlohmann::json j;
    input_stream >> j;
    texture_serializer::deserialize(j, obj);

    if (!load_img(obj->img_file_name, obj->width, obj->height, obj))
    {
      LOG_ERROR("Failed to load texture file: {0}", obj->img_file_name.c_str());
      return nullptr;
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
}