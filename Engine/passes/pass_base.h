#pragma once

#include <wrl/client.h>

#include "assets/material.h"

struct ID3D11InputLayout;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fvertex_shader_render_state;
  struct fpixel_shader_render_state;
  struct fscene_acceleration;
  class hscene;
  class hhittable_base;
  
  struct fpass_base
  {
    virtual ~fpass_base() = default;
    
    virtual void init() = 0;
    virtual void draw() = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;

    // Input
    const fpixel_shader_render_state* pixel_shader = nullptr;
    const fvertex_shader_render_state* vertex_shader = nullptr;
    const fscene_acceleration* scene_acceleration = nullptr;
    const hscene* scene = nullptr;
    const hhittable_base* selected_object = nullptr;
    int output_width = 1920;
    int output_height = 1080;
    fsoft_asset_ptr<amaterial> default_material_asset;

    // Basic DX11 objects
    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11SamplerState> sampler_state;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
  };
}