#include <fstream>
#include <sstream>

#include "asset/asset_io.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/resources_io.h"

#include "persistence/assets_json.h"
#include "nlohmann/json.hpp"
#include "object/object_registry.h"


namespace engine
{
  material_asset* asset_io::load_material_asset(const std::string& material_name)
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

    /*nlohmann::json j;
    input_stream >> j;
    material_type type = material_type::none;
    if (!TRY_PARSE(material_type, j, "type", type))
    {
      return nullptr;
    }*/

    material_asset* obj = material_asset::spawn();
    //obj->type = type;
    //if (obj == nullptr)
    //{
    //  LOG_ERROR("Invalid material type: {0}", static_cast<int>(type));
    //}

    nlohmann::json j;
    input_stream >> j;
    material_serializer::deserialize(j, obj);

    get_object_registry()->add<material_asset>(obj, material_name);

    return obj;
  }

  static_mesh_asset* asset_io::load_static_mesh_asset(const std::string& mesh_name)
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

    static_mesh_asset* obj = static_mesh_asset::spawn(); 
    /*if (obj == nullptr)
    {
      LOG_ERROR("Failed to spawn mesh object.");
      return nullptr;
    }*/
    
    nlohmann::json j;
    input_stream >> j;
    mesh_serializer::deserialize(j, obj);

    if (!load_obj(obj->obj_file_name, obj->shape_index, obj->faces))
    {
      LOG_ERROR("Failed to load object file: {0}", obj->obj_file_name.c_str());
      return nullptr;
    }

    get_object_registry()->add<static_mesh_asset>(obj, mesh_name);

    return obj;
  }

  texture_asset* asset_io::load_texture_asset(const std::string& texture_name)
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

    texture_asset* obj = texture_asset::spawn();
    //if (obj == nullptr)
    //{
    //  LOG_ERROR("Failed to spawn texture object.");
    //  return nullptr;
    //}

    nlohmann::json j;
    input_stream >> j;
    texture_serializer::deserialize(j, obj);

    if (!load_img(obj->img_file_name, obj->width, obj->height, obj))
    {
      LOG_ERROR("Failed to load texture file: {0}", obj->img_file_name.c_str());
      return nullptr;
    }

    get_object_registry()->add<texture_asset>(obj, texture_name);

    return obj;
  }

  void asset_io::save_material_asset(const material_asset* object)
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
}