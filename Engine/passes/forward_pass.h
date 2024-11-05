#pragma once

#include <wrl/client.h>

#include "passes/pass_base.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fforward_pass : public fpass_base
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
    std::vector<uint32_t> frame_data_heap_index;  // index is back buffer id
    std::vector<uint32_t> lights_data_heap_index;
    std::vector<uint32_t> materials_data_heap_index;
    uint32_t default_texture_heap_index = 0;
  };
}
