#pragma once

#include <string>
#include <wrl/client.h>

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "object/object.h"
#include "renderer/render_context.h"

struct ID3D12GraphicsCommandList;

namespace engine
{
  class hscene;
  class fwindow;
  struct fshader_render_state;
  
  // The responsibility of this class is to render to a texture
  class ENGINE_API rrenderer_base : public oobject
  {
  public:
    OBJECT_DECLARE(rrenderer_base, oobject)

    rrenderer_base();
    rrenderer_base(const rrenderer_base&) = delete;
    rrenderer_base& operator=(const rrenderer_base&) = delete;
    rrenderer_base(rrenderer_base&&) = delete;
    rrenderer_base& operator=(rrenderer_base&&) = delete;
    virtual ~rrenderer_base() override;
    
    frenderer_context context;
    
    uint32_t last_frame_resolution_hash = 0;
    int show_object_id = 0;

    // Main public interface
    void draw(ComPtr<ID3D12GraphicsCommandList> command_list, fwindow* in_window, hscene* in_scene, const hhittable_base* in_selected_object = nullptr);
    
  protected:
    virtual bool can_draw();
    virtual void init() = 0;
    virtual void draw_internal(ComPtr<ID3D12GraphicsCommandList> command_list) = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
    
  private:
    bool init_done = false;
  };
}
