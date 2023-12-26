#pragma once

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"

struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace engine
{
  class ENGINE_API gpu_renderer : public renderer_base
  {
  public:
    OBJECT_DECLARE(gpu_renderer, renderer_base)
    
    // FIX make them persistent members
    soft_asset_ptr<pixel_shader_asset> pixel_shader;
    soft_asset_ptr<vertex_shader_asset> vertex_shader;

    virtual bool is_async() const override { return false; }

    void setup_output_texture(unsigned int width, unsigned int height);
    void cleanup_output_texture();
    
    // Output texture, renders the scene there
    ID3D11RenderTargetView* output_rtv = nullptr;
    ID3D11ShaderResourceView* output_srv = nullptr;
    ID3D11Texture2D* output_texture = nullptr;
    unsigned int output_width = 0;
    unsigned int output_height = 0;
    
    ID3D11InputLayout* input_layout;
    ID3D11Buffer* vertex_buffer;
    unsigned int num_verts;
    unsigned int stride;
    unsigned int offset;
    
  private:
    virtual void job_init() override;
    virtual void job_update() override;
    virtual void job_cleanup() override;
  };
}
