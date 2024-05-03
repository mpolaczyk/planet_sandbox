#pragma once

#include <unordered_map>
#include <wrl/client.h>

#include "core/core.h"
#include "renderer/renderer_base.h"
#include "asset/soft_asset_ptr.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "assets/texture.h"
#include "assets/mesh.h"

struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;

#define MAX_LIGHTS 8

namespace engine
{
  class hstatic_mesh;
  
  using namespace DirectX;
  using Microsoft::WRL::ComPtr;
  
  class ENGINE_API rgpu : public rrenderer_base
  {    
  public:
    OBJECT_DECLARE(rgpu, rrenderer_base)
    
    // FIX make them persistent members
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;
    fsoft_asset_ptr<atexture> texture_asset;
    fsoft_asset_ptr<astatic_mesh> mesh_asset;
    
  protected:
    virtual void init() override;
    virtual void render_frame_impl() override;
    
  private:
    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11ShaderResourceView> texture_srv;
    ComPtr<ID3D11SamplerState> sampler_state;
    ComPtr<ID3D11Buffer> frame_constant_buffer;
    ComPtr<ID3D11Buffer> object_constant_buffer;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;

    // Rebuilt every frame based on hstatic_mesh objects placed on the scene
    // BEGIN
    
    // Map material pointer to material id (sent to gpu)
    // Needs to be quick to search
    typedef std::unordered_map<const amaterial*, uint32_t> material_map_type;
    material_map_type material_map;
    // Order of materials added to the material_map
    std::vector<const amaterial*> material_order;
    uint32_t next_material_id = 0;

    // All static meshes on the scene
    std::vector<const hstatic_mesh*> meshes;
    
    //END
  };
}
