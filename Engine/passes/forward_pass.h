#pragma once

#include <wrl/client.h>

#include "passes/pass_base.h"

struct ID3D12RootSignature;
struct ID3D12PipelineState;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fforward_pass : fpass_base
  {
    virtual void init() override;
    virtual void draw(const ComPtr<ID3D12GraphicsCommandList>& command_list) override;
    virtual void create_output_texture(bool cleanup = false) override;
    
    // Input
    int show_emissive = 1;
    int show_ambient = 1;
    int show_specular = 1;
    int show_diffuse = 1;
    int show_normals = 0;
    int show_object_id = 0;
    
    // private:
    //   ComPtr<ID3D11Buffer> frame_constant_buffer;
    //   ComPtr<ID3D11Buffer> object_constant_buffer;

    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<ID3D12PipelineState> pipeline_state;

    
    

    ComPtr<ID3D12Resource> index_buffer;

    ComPtr<ID3DBlob> vertex_shader_blob;
    ComPtr<ID3DBlob> pixel_shader_blob;
  };
}
