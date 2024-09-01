#pragma once

#include <wrl/client.h>

#include "passes/pass_base.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fforward_pass : fpass_base
  {
    virtual void init() override;
    virtual void draw(ComPtr<ID3D12GraphicsCommandList> command_list) override;
    virtual void create_output_texture(bool cleanup = false) override;
    
    // Input
    int show_emissive = 1;
    int show_ambient = 1;
    int show_specular = 1;
    int show_diffuse = 1;
    int show_normals = 0;
    int show_object_id = 0;
    
    private:
    
    ComPtr<ID3DBlob> vertex_shader_blob;
    ComPtr<ID3DBlob> pixel_shader_blob;
    std::vector<ComPtr<ID3D12Resource>> cbv;  // index is back buffer id
  };
}
