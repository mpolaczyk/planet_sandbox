
#include <sstream>

#include "asset/materials.h"

namespace engine
{
  asset_type material::get_static_asset_type()
  {
    return asset_type::material;
  }

  material* material::load(const std::string& material_name)
  {
    //return asset_discovery::load_material(material_name); // FIX
    return nullptr;
  }

  void material::save(material* object)
  {
    //asset_discovery::save_material(object);// FIX
  }

  material* material::spawn()
  {
    //return object_factory::spawn_material(material_type::none); // FIX
    return nullptr;
  }

  std::string material::get_display_name() const
  {
    std::ostringstream oss;
    oss << asset::get_display_name() << " - " << material_type_names[(int)type];
    return oss.str();
  }
}