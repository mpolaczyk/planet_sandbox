
#include "asset/soft_asset_ptr.cpp"

#include "asset/mesh.h"
#include "asset/materials.h"
#include "asset/textures.h"

// Explicit instantiations for the DLL client
// https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file

namespace engine
{
  template struct soft_asset_ptr<mesh>;
  template struct soft_asset_ptr<material>;
  template struct soft_asset_ptr<texture>;
}