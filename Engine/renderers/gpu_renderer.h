#pragma once

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"

struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;

namespace engine
{
  class ENGINE_API gpu_renderer : public renderer_base
  {
  public:
    OBJECT_DECLARE(gpu_renderer, renderer_base)
    
    // FIX make them persistent members
    soft_asset_ptr<pixel_shader_asset> pixel_shader;
    soft_asset_ptr<vertex_shader_asset> vertex_shader;

    virtual void render_frame(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config) override;
    virtual void push_partial_update() override {}
    virtual void cancel() override {}
    virtual bool is_async() const override { return false; }
    virtual bool is_working() const override { return true; }
    virtual bool is_cancelled() const override { return false; }
    virtual void cleanup() override;
        
    // Output texture, renders the scene there
    ID3D11RenderTargetView* output_rtv = nullptr;
    unsigned int output_width = 0;
    unsigned int output_height = 0;
    
    ID3D11InputLayout* input_layout;
    ID3D11Buffer* vertex_buffer;
    unsigned int num_verts;
    unsigned int stride;
    unsigned int offset;

    bool init_done = false;
    
  protected:
    void create_output_texture();
    void init();
    void update_frame();
  };
}
