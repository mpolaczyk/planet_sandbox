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
    struct fframe_data
    {
      XMFLOAT4X4 view_projection;
    };

    struct fobject_data
    {
      XMFLOAT4X4 model_world;                    // Used to transform the vertex position from object space to world space
      XMFLOAT4X4 inverse_transpose_model_world;  // Used to transform the vertex normal from object space to world space
      XMFLOAT4X4 model_world_view_projection;    // Used to transform the vertex position from object space to projected clip space
    };

    //struct light_data
    //{
    //  XMFLOAT4 eye_position;                // 16    16 - 1
    //  XMFLOAT4 global_ambient;              // 16    32 - 2
    //  flight_properties lights[MAX_LIGHTS];  // 8*80 672 - 42
    //};
    
  public:
    OBJECT_DECLARE(rgpu, rrenderer_base)
    
    // FIX make them persistent members
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;
    fsoft_asset_ptr<atexture> texture_asset;
    fsoft_asset_ptr<astatic_mesh> mesh_asset;
    
    virtual void render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera_config& in_camera_config) override;
    fcamera_config camera;
    const hscene* scene = nullptr;
    virtual void push_partial_update() override {}
    virtual void cancel() override {}
    virtual bool is_async() const override { return false; }
    virtual bool is_working() const override { return true; }
    virtual bool is_cancelled() const override { return false; }
    virtual void destroy() override;
        
    // Output texture, renders the scene there
    ComPtr<ID3D11RenderTargetView> output_rtv;
    ComPtr<ID3D11DepthStencilView> output_dsv;
    unsigned int output_width = 0;
    unsigned int output_height = 0;
    
    ComPtr<ID3D11InputLayout> input_layout;

    ComPtr<ID3D11ShaderResourceView> texture_srv;
    ComPtr<ID3D11SamplerState> sampler_state;

    ComPtr<ID3D11Buffer> frame_constant_buffer;
    ComPtr<ID3D11Buffer> object_constant_buffer;
    //ComPtr<ID3D11Buffer> material_constant_buffer;
    //ComPtr<ID3D11Buffer> light_constant_buffer;
    
    int64_t timestamp_start = 0;
    int64_t perf_counter_frequency = 0;
    double current_time = 0.0;  // [s]

    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;

    bool init_done = false;
    
  protected:
    void create_output_texture(bool cleanup = false);
    void init();
    void update_frame();
  };
}
