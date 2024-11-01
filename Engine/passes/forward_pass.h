#pragma once

#include <wrl/client.h>

#include "dxcapi.h"

#include "passes/pass_base.h"
#include "renderer/resource.h"

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
    ComPtr<IDxcBlob> vertex_shader_blob;
    ComPtr<IDxcBlob> pixel_shader_blob;

  private:
    
    std::vector<fresource> cbv_frame_resource;  // index is back buffer id
    std::vector<fresource> srv_lights_resource;
    std::vector<fresource> srv_materials_resource;
    fresource srv_first_texture;
  };
}
