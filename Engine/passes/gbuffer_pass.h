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
    ComPtr<ID3D11Texture2D> textures[egbuffer_type::count];
    ComPtr<ID3D11RenderTargetView> rtvs[egbuffer_type::count];
    ComPtr<ID3D11ShaderResourceView> srvs[egbuffer_type::count];
    ComPtr<ID3D11Texture2D> dsb;  // TODO not used! depth stencil buffer
    ComPtr<ID3D11DepthStencilView> dsv;

  private:
    ComPtr<ID3D11Buffer> object_constant_buffer;
  };
}
