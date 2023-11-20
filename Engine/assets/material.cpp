
#include <sstream>

#include "assets/material.h"

#include "asset/asset_io.h"

namespace engine
{
  OBJECT_DEFINE_BASE(material_asset)
  OBJECT_DEFINE_SAVE(material_asset)
  OBJECT_DEFINE_LOAD(material_asset)

  std::string material_asset::get_display_name() const
  {
    std::ostringstream oss;
    oss << object::get_display_name() << " - " << material_type_names[(int)type];
    return oss.str();
  }
}