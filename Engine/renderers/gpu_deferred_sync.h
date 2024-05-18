#pragma once


#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"

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

  class ENGINE_API rgpu_deferred_sync : public rrenderer_base
  {
  public:
    OBJECT_DECLARE(rgpu_deferred_sync, rrenderer_base)
    OBJECT_DECLARE_VISITOR

  protected:
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
  };
}