﻿#pragma once


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

  enum ebuffer_type
  {
    position,
    normal,
    tex_color,
    material_id,
    count,
  };

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

    // Runtime members
    int show_normal_ws = 0;
    int show_position_ws = 0;
    int show_tex_color = 0;
    
  protected:
    virtual bool can_render() const override;
    virtual void init() override;
    virtual void render_frame_impl() override;
    virtual void create_output_texture(bool cleanup = false) override;
    
  private:

    ComPtr<ID3D11ShaderResourceView> texture_srv;
    ComPtr<ID3D11SamplerState> sampler_state;
    ComPtr<ID3D11Buffer> lighting_deferred_frame_buffer;
    ComPtr<ID3D11Buffer> gbuffer_deferred_object_buffer;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;

    // GBuffer pass
    ComPtr<ID3D11InputLayout> gbuffer_input_layout;
    ComPtr<ID3D11Texture2D> gbuffer_textures[ebuffer_type::count];
    ComPtr<ID3D11RenderTargetView> gbuffer_rtvs[ebuffer_type::count];
    ComPtr<ID3D11ShaderResourceView> gbuffer_srvs[ebuffer_type::count];
    ComPtr<ID3D11Texture2D> gbuffer_dsb;
    ComPtr<ID3D11DepthStencilView> gbuffer_dsv;

    // Lighting pass
    ComPtr<ID3D11InputLayout> lighting_input_layout;
    fstatic_mesh_render_state quad_render_state;
    
    fscene_acceleration scene_acceleration;
  };
}