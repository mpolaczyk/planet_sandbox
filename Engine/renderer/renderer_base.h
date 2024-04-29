#pragma once

#include <string>
#include <wrl/client.h>

#include "renderer_config.h"
#include "resources/bmp.h"
#include "math/camera.h"
#include "object/object.h"

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace engine
{
  class hscene;
  
  // The responsibility of this class is to render to a texture
  class ENGINE_API rrenderer_base : public oobject
  {
  public:
    OBJECT_DECLARE(rrenderer_base, oobject)

    rrenderer_base() = default;
    rrenderer_base(const rrenderer_base&) = delete;
    rrenderer_base& operator=(const rrenderer_base&) = delete;
    rrenderer_base(rrenderer_base&&) = delete;
    rrenderer_base& operator=(rrenderer_base&&) = delete;

    fcamera camera;
    const hscene* scene = nullptr;
    
    // Output texture
    ComPtr<ID3D11Texture2D> output_texture;
    ComPtr<ID3D11ShaderResourceView> output_srv;
    ComPtr<ID3D11RenderTargetView> output_rtv;
    ComPtr<ID3D11DepthStencilView> output_dsv;
    ComPtr<ID3D11Texture2D> output_depth_texture;
    unsigned int output_width = 0;
    unsigned int output_height = 0;
    
    // Main public interface
    void render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera& in_camera_config);
    double get_render_time_ms() const { return render_time_ms; }

  protected:
    virtual void init() = 0;
    virtual void render_frame_impl() = 0;
    double render_time_ms = 0.0;
  
  private:
    void create_output_texture(bool cleanup = false);
    bool init_done = false;
  };
}
