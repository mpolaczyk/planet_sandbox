#include "stdafx.h"

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
  material_serializer::serialize(object);
}

material* material::spawn()
{
  return object_factory::spawn_material(material_type::none);
}