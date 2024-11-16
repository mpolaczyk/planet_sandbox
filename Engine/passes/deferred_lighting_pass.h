#pragma once

#include <wrl/client.h>

#include "passes/gbuffer_type.h"
#include "passes/pass_base.h"

//struct ID3D11Texture2D;
//struct ID3D11Buffer;
//struct ID3D11RenderTargetView;
//struct ID3D11ShaderResourceView;
//struct ID3D11DepthStencilView;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fdeferred_lighting_pass : public fpass_base
  {
    virtual void init() override;
    virtual void draw(fgraphics_command_list* command_list) override;
    virtual void init_size_dependent(bool cleanup) override;

    // Input
    int show_normal_ws = 0;
    int show_position_ws = 0;
    int show_tex_color = 0;
    int show_object_id = 0;
    //    ComPtr<ID3D11ShaderResourceView> gbuffer_srvs[egbuffer_type::count];
    //    
    //    // Output
    //    ComPtr<ID3D11Texture2D> output_texture;
    //    ComPtr<ID3D11Texture2D> output_depth;
    //    ComPtr<ID3D11ShaderResourceView> output_srv;
    //    ComPtr<ID3D11RenderTargetView> output_rtv;
    //    ComPtr<ID3D11DepthStencilView> output_dsv;
    //
    //  private:
    //    ComPtr<ID3D11Buffer> frame_constant_buffer;
    fstatic_mesh_resource quad_render_state;
  };
}