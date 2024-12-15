#include "engine/asset/soft_asset_ptr.cpp"

// FIX Replace with forward declarations, see the comment in the soft_asset_ptr class
#include "assets/mesh.h"
#include "assets/material.h"
#include "assets/texture.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"

// Explicit instantiations for the DLL client
// https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file

namespace engine
{
  template struct fsoft_asset_ptr<astatic_mesh>;
  template struct fsoft_asset_ptr<amaterial>;
  template struct fsoft_asset_ptr<atexture>;

  template struct fsoft_asset_ptr<ashader>;
  template struct fsoft_asset_ptr<apixel_shader>;
  template struct fsoft_asset_ptr<avertex_shader>;
}
