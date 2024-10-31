#pragma once

#include <wrl/client.h>
#include <dxcapi.h>

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
    ComPtr<IDxcBlob> vertex_shader_blob;
    ComPtr<IDxcBlob> pixel_shader_blob;

  private:

    std::vector<ComPtr<ID3D12Resource>> cbv_frame_resource;  // index is back buffer id
    std::vector<ComPtr<ID3D12Resource>> srv_lights_resource;
    std::vector<ComPtr<ID3D12Resource>> srv_materials_resource;

    // Main heap structure
    uint32_t cbv_frame_data_heap_index = 0;
    uint32_t srv_lights_heap_index = 0;
    uint32_t srv_materials_heap_index = 0;
    uint32_t srv_textures_heap_index = 0;
  };
}
