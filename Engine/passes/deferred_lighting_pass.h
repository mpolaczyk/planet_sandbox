#pragma once

#include <wrl/client.h>

#include "passes/pass_base.h"

namespace engine
{
  class astatic_mesh;
  
  using Microsoft::WRL::ComPtr;
  
  struct fdeferred_lighting_pass : public fpass_base
  {
    virtual void init() override;
    virtual void draw(fgraphics_command_list* command_list) override;
    virtual void init_size_dependent(bool cleanup) override;

    // Input
    int show_position = 0;
    int show_normal = 0;
    int show_uv = 0;
    int show_material_id = 0;
    // GBuffer
    ftexture_resource* position;
    ftexture_resource* normal;
    ftexture_resource* uv;
    ftexture_resource* material_id;

    // Output
    ftexture_resource color;

  private:
    std::vector<fconst_buffer> frame_data; // index is back buffer id
    std::vector<fshader_resource_buffer> lights_data;
    std::vector<fshader_resource_buffer> materials_data;
    std::vector<ftexture_resource> textures_data;
    astatic_mesh* quad_mesh = nullptr;  // TODO fix the leak
    bool is_gbuffer_on_the_heap = false;
  };
}