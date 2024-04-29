
#include <d3d11_1.h>

#include "renderer/dx11_lib.h"
#include "renderer/renderer_base.h"
#include "engine/log.h"
#include "object/object_registry.h"
#include "profile/benchmark.h"

namespace engine
{
  OBJECT_DEFINE(rrenderer_base, oobject, Renderer base)
  OBJECT_DEFINE_NOSPAWN(rrenderer_base) 

  void rrenderer_base::render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera& in_camera_config)
  {
    if (in_renderer_config.resolution_vertical == 0 || in_renderer_config.resolution_horizontal == 0) return;
    camera = in_camera_config;
    scene = in_scene;

    const bool size_changed = output_width != in_renderer_config.resolution_horizontal
                          || output_height != in_renderer_config.resolution_vertical;
    output_width = in_renderer_config.resolution_horizontal;
    output_height = in_renderer_config.resolution_vertical;
    if (size_changed)
    {
      LOG_INFO("Recreating output texture");
      create_output_texture(true);
    }
    
    // Initialize
    if (!init_done)
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

  void rrenderer_base::create_output_texture(bool cleanup)
    {
        if (cleanup)
        {
            DX_RELEASE(output_rtv)
            DX_RELEASE(output_srv)
            DX_RELEASE(output_dsv)
            DX_RELEASE(output_texture)
            DX_RELEASE(output_depth_texture)
        }
        
        fdx11& dx = fdx11::instance();
        
        // Output and depth texture
        {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width = output_width;
            desc.Height = output_height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            if(FAILED(dx.device->CreateTexture2D(&desc, NULL, &output_texture)))
            {
                throw std::runtime_error("CreateTexture2D output texture failed.");
            }
            desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            if(FAILED(dx.device->CreateTexture2D(&desc, NULL, &output_depth_texture)))
            {
                throw std::runtime_error("CreateTexture2D output depth texture failed.");
            }
        }
        // RTV
        {
            D3D11_RENDER_TARGET_VIEW_DESC desc = {};
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
            if(FAILED(dx.device->CreateRenderTargetView(output_texture.Get(), &desc, output_rtv.GetAddressOf())))
            {
                throw std::runtime_error("CreateRenderTargetView output texture failed");
            }
        }
        // SRV
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip = 0;
            desc.Texture2D.MipLevels = 1;
            if(FAILED(dx.device->CreateShaderResourceView(output_texture.Get(), &desc, output_srv.GetAddressOf())))
            {
                throw std::runtime_error("CreateShaderResourceView output texture failed.");
            }
        }
        // DSV
        {
            if(FAILED(dx.device->CreateDepthStencilView(output_depth_texture.Get(), nullptr, output_dsv.GetAddressOf())))
            {
                throw std::runtime_error("CreateDepthStencilView output depth texture failed");
            }
        }
    }
}
