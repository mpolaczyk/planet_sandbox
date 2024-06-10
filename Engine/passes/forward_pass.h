#pragma once

#include <wrl/client.h>

#include "passes/pass_base.h"

struct ID3D11Texture2D;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fforward_pass : fpass_base
  {
    virtual void init() override;
    virtual void draw() override;
    virtual void create_output_texture(bool cleanup = false) override;
    
    // Input
    int show_emissive = 1;
    int show_ambient = 1;
    int show_specular = 1;
    int show_diffuse = 1;
    int show_normals = 0;
    int show_object_id = 0;
    
    // Output
    ComPtr<ID3D11Texture2D> output_texture;
    ComPtr<ID3D11Texture2D> output_depth;
    ComPtr<ID3D11ShaderResourceView> output_srv;
    ComPtr<ID3D11RenderTargetView> output_rtv;
    ComPtr<ID3D11DepthStencilView> output_dsv;

  private:
    ComPtr<ID3D11Buffer> frame_constant_buffer;
    ComPtr<ID3D11Buffer> object_constant_buffer;
  };
}
