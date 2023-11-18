
#include <sstream>

#include "asset/materials.h"

#include "asset/asset_discovery.h"

namespace engine
{
  object_type material::get_static_type()
  {
    return object_type::material;
  }

  material* material::load(const std::string& material_name)
  {
    return asset_discovery::load_material(material_name);
  }

  void material::save(material* object)
  {
    asset_discovery::save_material(object);
  }

  material* material::spawn()
  {
    return new material(material_type::none);
  }

  std::string material::get_display_name() const
  {
    std::ostringstream oss;
    oss << object::get_display_name() << " - " << material_type_names[(int)type];
    return oss.str();
  }
}