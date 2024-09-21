#pragma once

#include <wrl/client.h>

#include "assets/material.h"

//struct ID3D11InputLayout;
//struct ID3D11SamplerState;
//struct ID3D11RasterizerState;
//struct ID3D11DepthStencilState;

struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fvertex_shader_render_state;
  struct fpixel_shader_render_state;
  struct fscene_acceleration;
  class fwindow;
  
  class hscene;
  class hhittable_base;
  
  struct fpass_base
  {
    virtual ~fpass_base() = default;
    
    virtual void init() = 0;
    virtual void draw(ComPtr<ID3D12GraphicsCommandList> command_list) = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
    virtual void copy_input(fwindow* in_window, const fvertex_shader_render_state* in_vertex_shader, const fpixel_shader_render_state* in_pixel_shader,
      fscene_acceleration* in_scene_acceleration, const hscene* in_scene, const hhittable_base* in_selected_object,
      int in_output_width, int in_output_height,
      fsoft_asset_ptr<amaterial>& in_default_material)
    {
      window = in_window;
      pixel_shader = in_pixel_shader;
      vertex_shader = in_vertex_shader;
      scene_acceleration = in_scene_acceleration;
      scene = in_scene;
      selected_object = in_selected_object;
      output_width = in_output_width;
      output_height = in_output_height;
      default_material_asset = in_default_material;
    }
    bool get_can_render() const { return can_render; }
    
  protected:
    // Input
    const fpixel_shader_render_state* pixel_shader = nullptr;
    const fvertex_shader_render_state*  vertex_shader = nullptr;
    fscene_acceleration* scene_acceleration = nullptr;
    const hscene* scene = nullptr;
    const hhittable_base* selected_object = nullptr;
    fwindow* window = nullptr;
    int output_width = 1920;
    int output_height = 1080;
    fsoft_asset_ptr<amaterial> default_material_asset;
    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<ID3D12PipelineState> pipeline_state;

    bool can_render = true;
  };
}