#pragma once

#include <DirectXMath.h>

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

using namespace DirectX;

#define MAX_LIGHTS 8

namespace engine
{
  class ENGINE_API gpu_renderer : public renderer_base
  {
    struct per_frame_data
    {
      XMFLOAT4X4 model_view_proj;
    };

    struct per_object_data
    {
      XMMATRIX world;                    // Used to transform the vertex position from object space to world space
      XMMATRIX inverse_transpose_world;  // Used to transform the vertex normal from object space to world space
      XMMATRIX world_view_projection;    // Used to transform the vertex position from object space to projected clip space
    };

    struct light_data
    {
      XMFLOAT4 eye_position;                // 16    16 - 1
      XMFLOAT4 global_ambient;              // 16    32 - 2
      light_properties lights[MAX_LIGHTS];  // 8*80 672 - 42
    };
    
  public:
    OBJECT_DECLARE(gpu_renderer, renderer_base)
    
    // FIX make them persistent members
    soft_asset_ptr<pixel_shader_asset> pixel_shader_asset;
    soft_asset_ptr<vertex_shader_asset> vertex_shader_asset;
    soft_asset_ptr<texture_asset> texture_asset;
    soft_asset_ptr<static_mesh_asset> mesh_asset;
    
    virtual void render_frame(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config) override;
    camera_config camera;
    const scene* scenee; // FIX Name is the same as the type, that creates issues :/ Add prefixes to types!
    virtual void push_partial_update() override {}
    virtual void cancel() override {}
    virtual bool is_async() const override { return false; }
    virtual bool is_working() const override { return true; }
    virtual bool is_cancelled() const override { return false; }
    virtual void cleanup() override;
        
    // Output texture, renders the scene there
    ID3D11RenderTargetView* output_rtv = nullptr;
    ID3D11DepthStencilView* output_dsv = nullptr;
    unsigned int output_width = 0;
    unsigned int output_height = 0;
    
    ID3D11InputLayout* input_layout;

    ID3D11ShaderResourceView* texture_srv;
    ID3D11SamplerState* sampler_state;

    ID3D11Buffer* per_frame_constant_buffer;
    ID3D11Buffer* per_object_constant_buffer;
    ID3D11Buffer* material_constant_buffer;
    ID3D11Buffer* light_constant_buffer;
    
    int64_t timestamp_start = 0;
    int64_t perf_counter_frequency = 0;
    double current_time = 0.0;  // [s]

    ID3D11RasterizerState* rasterizer_state;
    ID3D11DepthStencilState* depth_stencil_state;

    bool init_done = false;
    
  protected:
    void create_output_texture(bool cleanup = false);
    void init();
    void update_frame();
  };
}
