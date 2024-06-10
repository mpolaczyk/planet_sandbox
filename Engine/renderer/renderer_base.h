#pragma once

#include <string>
#include <wrl/client.h>

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "resources/bmp.h"
#include "object/object.h"
#include "renderer/scene_acceleration.h"

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

    const hscene* scene = nullptr;
    const hhittable_base* selected_object = nullptr;

    fscene_acceleration scene_acceleration;
    
    uint32_t last_frame_resolution_hash = 0;
    int show_object_id = 0;

    // Persistent members
    fsoft_asset_ptr<amaterial> default_material_asset;
    int output_width = 1920;
    int output_height = 1080;

    // Main public interface
    void render_frame(const hscene* in_scene, const hhittable_base* in_selected_object = nullptr);
    double get_render_time_ms() const { return render_time_ms; }
    virtual ComPtr<ID3D11Texture2D> get_output_texture() const = 0;
    virtual ComPtr<ID3D11Texture2D> get_output_depth() const = 0;
    virtual ComPtr<ID3D11ShaderResourceView> get_output_srv() const = 0;
    virtual ComPtr<ID3D11RenderTargetView> get_output_rtv() const = 0;
    virtual ComPtr<ID3D11DepthStencilView> get_output_dsv() const = 0;
    
  protected:
    virtual bool can_render();
    virtual void init() = 0;
    virtual void render_frame_impl() = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
    
    double render_time_ms = 0.0;
    
  private:
    bool init_done = false;
  };
}
