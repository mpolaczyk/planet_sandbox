#pragma once

#include <wrl/client.h>

#include "hittables/static_mesh.h"
#include "passes/pass_base.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fdeferred_lighting_pass : public fpass_base
  {
    virtual void init_pipeline() override;
    virtual void init_size_independent_resources() override;
    virtual void init_size_dependent_resources(bool cleanup) override;
    virtual void draw(fgraphics_command_list* command_list) override;
    
    // Input: GBuffer
    ftexture_resource* position;
    ftexture_resource* normal;
    ftexture_resource* uv;
    ftexture_resource* material_id;

    // Output
    ftexture_resource color;

  private:
    fframe_data frame_data{};
    std::vector<fshader_resource_buffer> lights_data;
    std::vector<fshader_resource_buffer> materials_data;
    std::vector<ftexture_resource> textures_data;
    fsoft_asset_ptr<astatic_mesh> quad_asset;
  };
}