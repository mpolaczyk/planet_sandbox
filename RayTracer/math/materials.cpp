#include "stdafx.h"

#include <fstream>

#include "app/factories.h"
#include "app/asset_discovery.h"
#include "app/json/assets_json.h"

#include "math/materials.h"

asset_type material::get_static_asset_type() 
{ 
  return asset_type::material; 
}

material* material::load(const std::string& material_name)
{
  return asset_discovery::load_material(material_name);
}

void material::save(material* object)
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

material* material::spawn()
{
  return object_factory::spawn_material(material_type::none);
}