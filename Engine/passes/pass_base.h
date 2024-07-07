#pragma once

#include <wrl/client.h>

#include "assets/material.h"

//struct ID3D11InputLayout;
//struct ID3D11SamplerState;
//struct ID3D11RasterizerState;
//struct ID3D11DepthStencilState;

struct ID3D12GraphicsCommandList;

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
    virtual void draw(const ComPtr<ID3D12GraphicsCommandList>& command_list) = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
    virtual void copy_input(const fvertex_shader_render_state* in_vertex_shader, const fpixel_shader_render_state* in_pixel_shader,
      fscene_acceleration* in_scene_acceleration, const hscene* in_scene, const hhittable_base* in_selected_object,
      int in_output_width, int in_output_height,
      fsoft_asset_ptr<amaterial>& in_default_material)
    {
      pixel_shader = in_pixel_shader;
      vertex_shader = in_vertex_shader;
      scene_acceleration = in_scene_acceleration;
      scene = in_scene;
      selected_object = in_selected_object;
      output_width = in_output_width;
      output_height = in_output_height;
      default_material_asset = in_default_material;
    }
    
  protected:
    // Input
    const fpixel_shader_render_state* pixel_shader = nullptr;
    const fvertex_shader_render_state* vertex_shader = nullptr;
    fscene_acceleration* scene_acceleration = nullptr;
    const hscene* scene = nullptr;
    const hhittable_base* selected_object = nullptr;
    int output_width = 1920;
    int output_height = 1080;
    fsoft_asset_ptr<amaterial> default_material_asset;

    // Basic DX11 objects
//    ComPtr<ID3D11InputLayout> input_layout;
//    ComPtr<ID3D11SamplerState> sampler_state;
//    ComPtr<ID3D11RasterizerState> rasterizer_state;
//    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
  };
}