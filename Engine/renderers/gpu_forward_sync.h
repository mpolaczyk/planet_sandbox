#pragma once

#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "renderers/scene_acceleration.h"

struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;

namespace engine
{
  class hstatic_mesh;
  class hlight;

  using namespace DirectX;
  using Microsoft::WRL::ComPtr;

  class ENGINE_API rgpu_forward_sync : public rrenderer_base
  {
  public:
    OBJECT_DECLARE(rgpu_forward_sync, rrenderer_base)
    OBJECT_DECLARE_VISITOR

    // Persistent members
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;

    // Runtime members
    int show_emissive = 1;
    int show_ambient = 1;
    int show_specular = 1;
    int show_diffuse = 1;
    int show_normals = 0;

  protected:
    virtual bool can_render() const override;
    virtual void init() override;
    virtual void render_frame_impl() override;

  private:
    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11ShaderResourceView> texture_srv;
    ComPtr<ID3D11SamplerState> sampler_state;
    ComPtr<ID3D11Buffer> frame_constant_buffer;
    ComPtr<ID3D11Buffer> object_constant_buffer;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;

    fscene_acceleration scene_acceleration;
  };
}
