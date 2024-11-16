#pragma once

#include <wrl/client.h>

#include "dxcapi.h"

#include "passes/pass_base.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fgbuffer_pass : public fpass_base
  {
    virtual void init() override;
    virtual void draw(fgraphics_command_list* command_list) override;
    virtual void init_size_dependent(bool cleanup) override;
    
    int show_object_id = 0;
    
    // Output
 //   ComPtr<ID3D11Texture2D> output_texture[egbuffer_type::count];
 //   ComPtr<ID3D11RenderTargetView> output_rtv[egbuffer_type::count];
 //   ComPtr<ID3D11ShaderResourceView> output_srv[egbuffer_type::count];
 //   ComPtr<ID3D11DepthStencilView> output_dsv;
 //   ComPtr<ID3D11Texture2D> output_depth;
 //   
 // private:
 //   ComPtr<ID3D11Buffer> object_constant_buffer;
  };
}
