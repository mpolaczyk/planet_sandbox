#pragma once

#include <DirectXMath.h>
#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "assets/texture.h"

struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;

#define MAX_LIGHTS 8

namespace engine
{
  using namespace DirectX;

  using Microsoft::WRL::ComPtr;
  
  class ENGINE_API rgpu : public rrenderer_base
  {
    
    struct fframe_data  // WARNING - const buffer type
    {
      XMFLOAT4X4 view_projection;
      XMFLOAT4 ambient_light;     // TODO alignment?
      flight_properties light;    // TEMP test implementation
    };

    struct fobject_data // WARNING - const buffer type
    {
      XMFLOAT4X4 model_world;                    // Used to transform the vertex position from object space to world space
      XMFLOAT4X4 inverse_transpose_model_world;  // Used to transform the vertex normal from object space to world space
      XMFLOAT4X4 model_world_view_projection;    // Used to transform the vertex position from object space to projected clip space
    };
    
  public:
    OBJECT_DECLARE(rgpu, rrenderer_base)
    
    // FIX make them persistent members
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;
    fsoft_asset_ptr<atexture> texture_asset;
    fsoft_asset_ptr<astatic_mesh> mesh_asset;
    
  protected:
    virtual void init() override;
    virtual void render_frame_impl() override;
    
  private:
    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11ShaderResourceView> texture_srv;
    ComPtr<ID3D11SamplerState> sampler_state;
    ComPtr<ID3D11Buffer> ps_frame_constant_buffer;
    ComPtr<ID3D11Buffer> vs_object_constant_buffer;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
  };
}
