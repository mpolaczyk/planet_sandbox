#pragma once

#include "core/core.h"
#include "asset/soft_asset_ptr.h"
#include "assets/material.h"

namespace engine
{
  class fwindow;
  class hscene;
  class hhittable_base;
  struct fshader_render_state;
  struct fscene_acceleration;
  
  struct ENGINE_API frenderer_context
  {
    const hhittable_base* selected_object = nullptr;          // weak ptr
    hscene* scene = nullptr;                                  // weak ptr
    fwindow* window = nullptr;                                // weak ptr
    fscene_acceleration* scene_acceleration = nullptr;        // unique ptr
    int output_width = 1920;
    int output_height = 1080;
    fsoft_asset_ptr<amaterial> default_material_asset;
  };
}
