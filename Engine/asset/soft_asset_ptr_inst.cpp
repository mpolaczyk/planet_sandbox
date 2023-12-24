
#include "asset/soft_asset_ptr.cpp"

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
  template struct soft_asset_ptr<static_mesh_asset>;
  template struct soft_asset_ptr<material_asset>;
  template struct soft_asset_ptr<texture_asset>;

  template struct soft_asset_ptr<pixel_shader_asset>;
  template struct soft_asset_ptr<vertex_shader_asset>;
}