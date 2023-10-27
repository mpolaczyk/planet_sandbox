
#include "asset/soft_asset_ptr.cpp"

#include "asset/mesh.h"
#include "asset/materials.h"
#include "asset/textures.h"

namespace engine
{
  template struct soft_asset_ptr<mesh>;
  template struct soft_asset_ptr<material>;
  template struct soft_asset_ptr<texture>;
}