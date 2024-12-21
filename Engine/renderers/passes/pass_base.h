#pragma once

#include "engine/unique_ptr.h"
#include "engine/asset/soft_asset_ptr.h"

struct ID3D12GraphicsCommandList;
struct CD3DX12_GPU_DESCRIPTOR_HANDLE;

namespace engine
{
  class apixel_shader;
  class avertex_shader;
  struct fgraphics_command_list;
  struct frenderer_context;
  struct fgraphics_pipeline;

  struct fpass_base
  {
    CTOR_VDTOR(fpass_base)
    CTOR_MOVE_COPY_DELETE(fpass_base)

    // Main interface
    bool init(frenderer_context* in_context);
    virtual void draw(frenderer_context* in_context, fgraphics_command_list* command_list);
    bool can_draw() const;

    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;
    
  protected:
    // Initialization flow
    virtual void init_shaders() = 0;
    virtual void init_pipeline();
    virtual void init_size_independent_resources() = 0;
    virtual void init_size_dependent_resources(bool cleanup) = 0;

    // Pass helpers
    void update_vertex_and_index_buffers(fgraphics_command_list* command_list) const;
    void upload_all_textures_once(fgraphics_command_list* command_list) const;
    CD3DX12_GPU_DESCRIPTOR_HANDLE get_textures_gpu_handle() const;
    
    frenderer_context* context = nullptr; // weak ptr, owned by renderer
    funique_ptr<fgraphics_pipeline> graphics_pipeline;
  };

}