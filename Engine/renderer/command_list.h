#pragma once

#include <cstdint>
#include <wrl/client.h>

#include "core/core.h"

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
enum D3D12_RESOURCE_STATES;

namespace engine
{
  class atexture;
  class astatic_mesh;
  struct fdescriptor_heap;
  struct ftexture_resource;
  
  using namespace Microsoft::WRL;
  
  struct ENGINE_API fgraphics_command_list final
  {
    void resource_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const;

    void set_render_targets(const ftexture_resource& rtv, const ftexture_resource* dsv = nullptr) const;
    void set_viewport(uint32_t width, uint32_t height) const;
    void set_scissor(uint32_t width, uint32_t height) const;

    void clear_render_target(const ftexture_resource& rtv) const;
    void clear_depth_stencil(const ftexture_resource& dsv) const;

    void upload_buffer_resource(uint64_t buffer_size, const void* in_buffer, ComPtr<ID3D12Resource>& out_upload_intermediate, ComPtr<ID3D12Resource>& out_gpu_resource) const;
    void upload_vertex_buffer(astatic_mesh* mesh, const char* name) const;
    void upload_index_buffer(astatic_mesh* mesh, const char* name) const;
    void upload_texture(atexture* texture_asset) const;
    
    ComPtr<ID3D12GraphicsCommandList> com;
  };
}
