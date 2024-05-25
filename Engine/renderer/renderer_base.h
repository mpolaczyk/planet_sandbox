#pragma once

#include <string>
#include <wrl/client.h>

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "resources/bmp.h"
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

    const hscene* scene = nullptr;
    const hhittable_base* selected_object = nullptr;
    uint32_t last_frame_resolution_hash = 0;
    int show_object_id = 0;

    // Output texture
    ComPtr<ID3D11Texture2D> output_texture;
    ComPtr<ID3D11Texture2D> output_depth;
    ComPtr<ID3D11ShaderResourceView> output_srv;
    ComPtr<ID3D11RenderTargetView> output_rtv;
    ComPtr<ID3D11DepthStencilView> output_dsv;

    // Persistent members
    fsoft_asset_ptr<amaterial> default_material_asset;
    int32_t output_width = 1920;
    int32_t output_height = 1080;

    // Main public interface
    void render_frame(const hscene* in_scene, const hhittable_base* in_selected_object = nullptr);
    double get_render_time_ms() const { return render_time_ms; }

  protected:
    virtual bool can_render() const;
    virtual void init() = 0;
    virtual void render_frame_impl() = 0;
    virtual void create_output_texture(bool cleanup = false);

    double render_time_ms = 0.0;
    
  private:
    bool init_done = false;
  };
}
