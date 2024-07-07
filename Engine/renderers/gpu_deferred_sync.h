#pragma once


#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "passes/deferred_lighting_pass.h"
#include "passes/gbuffer_pass.h"

//struct ID3D11InputLayout;
//struct ID3D11Buffer;
//struct ID3D11RenderTargetView;
//struct ID3D11ShaderResourceView;
//struct ID3D11DepthStencilView;
//struct ID3D11SamplerState;
//struct ID3D11RasterizerState;
//struct ID3D11DepthStencilState;

struct ID3D12GraphicsCommandList;

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

    // Persistent members
    fsoft_asset_ptr<apixel_shader> gbuffer_pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> gbuffer_vertex_shader_asset;
    fsoft_asset_ptr<apixel_shader> lighting_pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> lighting_vertex_shader_asset;

    //virtual ComPtr<ID3D11Texture2D> get_output_texture() const override { return deferred_lighting_pass.output_texture; };
    //virtual ComPtr<ID3D11Texture2D> get_output_depth() const override { return deferred_lighting_pass.output_depth; };
    //virtual ComPtr<ID3D11ShaderResourceView> get_output_srv() const override { return deferred_lighting_pass.output_srv; };
    //virtual ComPtr<ID3D11RenderTargetView> get_output_rtv() const override { return deferred_lighting_pass.output_rtv; };
    //virtual ComPtr<ID3D11DepthStencilView> get_output_dsv() const override { return deferred_lighting_pass.output_dsv; };

    fgbuffer_pass gbuffer_pass;
    fdeferred_lighting_pass deferred_lighting_pass;
    
  protected:
    virtual bool can_render() override;
    virtual void init() override;
    virtual void render_frame_impl(const ComPtr<ID3D12GraphicsCommandList>& command_list) override;
    virtual void create_output_texture(bool cleanup = false) override;
  };
}