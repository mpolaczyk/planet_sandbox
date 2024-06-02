#include <d3d11_1.h>

#include "renderer/dx11_lib.h"
#include "renderer/renderer_base.h"
#include "engine/log.h"
#include "engine/hash.h"
#include "hittables/scene.h"
#include "object/object_registry.h"
#include "profile/benchmark.h"

namespace engine
{
  OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
  OBJECT_DEFINE_NOSPAWN(rrenderer_base)

  void rrenderer_base::render_frame(const hscene* in_scene, const hhittable_base* in_selected_object)
  {
    scene = in_scene;
    selected_object = in_selected_object;

    if(!can_render())
    {
      return;
    }

    const uint32_t resolution_hash = fhash::combine(output_height, output_width);
    if(resolution_hash != last_frame_resolution_hash)
    {
      LOG_INFO("Recreating output texture");
      create_output_texture(true);
      last_frame_resolution_hash = resolution_hash;
    }

    // Initialize
    if(!init_done)
    {
      init();
      init_done = true;
    }

    uint64_t render_time_us;
    {
      fscope_timer benchmark_renderer("Render", &render_time_us);
      render_frame_impl();
    }
    render_time_ms = static_cast<double>(render_time_us) / 1000;
  }

  bool rrenderer_base::can_render()
  {
    if(output_height == 0 || output_width == 0)
    {
      LOG_ERROR("Can't render. Incorrect resolution.");
      return false;
    }
    if(scene == nullptr)
    {
      LOG_ERROR("Can't render. Scene is missing.");
      return false;
    }
    return true;
  }

  void rrenderer_base::create_output_texture(bool cleanup)
  {
    if(cleanup)
    {
      DX_RELEASE(output_rtv)
      DX_RELEASE(output_srv)
      DX_RELEASE(output_dsv)
      DX_RELEASE(output_texture)
      DX_RELEASE(output_depth)
    }

    fdx11& dx = fdx11::instance();
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    D3D11_BIND_FLAG bind_flag = static_cast<D3D11_BIND_FLAG>(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    dx.create_texture(output_width, output_height, format, bind_flag, D3D11_USAGE_DEFAULT, output_texture);
    dx.create_shader_resource_view(output_texture, format, D3D11_SRV_DIMENSION_TEXTURE2D, output_srv);
    dx.create_render_target_view(output_texture, format, D3D11_RTV_DIMENSION_TEXTURE2D, output_rtv);

    dx.create_texture(output_width, output_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, output_depth);
    dx.create_depth_stencil_view(output_depth, output_width, output_height, output_dsv);
  }
}