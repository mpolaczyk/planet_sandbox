#pragma once

#include <wrl/client.h>

#include "passes/gbuffer_type.h"
#include "passes/pass_base.h"

struct ID3D11Texture2D;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fgbuffer_pass : fpass_base
  {
    virtual void init() override;
    virtual void draw() override;
    virtual void create_output_texture(bool cleanup = false) override;
    
    // Output
    ComPtr<ID3D11Texture2D> output_texture[egbuffer_type::count];
    ComPtr<ID3D11RenderTargetView> output_rtv[egbuffer_type::count];
    ComPtr<ID3D11ShaderResourceView> output_srv[egbuffer_type::count];
    ComPtr<ID3D11DepthStencilView> output_dsv;
    ComPtr<ID3D11Texture2D> output_depth;
    
  private:
    ComPtr<ID3D11Buffer> object_constant_buffer;
  };
}
