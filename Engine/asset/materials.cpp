
#include <sstream>

#include "asset/materials.h"

#include "asset/asset_discovery.h"

namespace engine
{
  OBJECT_DEFINE_BASE(material)
  OBJECT_DEFINE_SAVE(material)
  OBJECT_DEFINE_LOAD(material)

  std::string material::get_display_name() const
  {
    std::ostringstream oss;
    oss << object::get_display_name() << " - " << material_type_names[(int)type];
    return oss.str();
  }
}