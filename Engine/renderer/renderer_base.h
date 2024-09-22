#pragma once

#include <string>
#include <wrl/client.h>

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "resources/bmp.h"
#include "object/object.h"
#include "renderer/scene_acceleration.h"

//struct ID3D11ShaderResourceView;
//struct ID3D11Texture2D;
struct ID3D12GraphicsCommandList;

namespace engine
{
  class hscene;
  class fwindow;
  
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

    hscene* scene = nullptr;
    const hhittable_base* selected_object = nullptr;
    fwindow* window = nullptr;

    fscene_acceleration scene_acceleration;
    
    uint32_t last_frame_resolution_hash = 0;
    int show_object_id = 0;

    // Persistent members
    fsoft_asset_ptr<amaterial> default_material_asset;
    int output_width = 1920;
    int output_height = 1080;

    // Main public interface
    void render_frame(ComPtr<ID3D12GraphicsCommandList> command_list, fwindow* in_window, hscene* in_scene, const hhittable_base* in_selected_object = nullptr);    // TODO rename to draw
    
  protected:
    virtual bool can_render();
    virtual void init() = 0;
    virtual void render_frame_internal(ComPtr<ID3D12GraphicsCommandList> command_list) = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
    
  private:
    bool init_done = false;
  };
}
