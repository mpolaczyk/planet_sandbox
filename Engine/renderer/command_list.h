#pragma once

#include <cstdint>
#include <vector>
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
    
    void set_render_targets1(ftexture_resource* render_target, const ftexture_resource* dsv) const;
    void set_render_targets(uint32_t num_render_targets, ftexture_resource** render_targets, const ftexture_resource* dsv) const;
    void set_viewport(uint32_t width, uint32_t height) const;
    void set_scissor(uint32_t width, uint32_t height) const;

    void clear_render_target(const ftexture_resource* rtv, const float color[4]) const;
    void clear_depth_stencil(const ftexture_resource* dsv) const;

    void upload_buffer_resource(uint32_t buffer_size, const void* in_buffer, ComPtr<ID3D12Resource>& out_upload_intermediate, ComPtr<ID3D12Resource>& out_gpu_resource) const;
    void upload_vertex_buffer(astatic_mesh* mesh, const char* name) const;
    void upload_index_buffer(astatic_mesh* mesh, const char* name) const;
    void upload_texture(atexture* texture_asset) const;
    
    ComPtr<ID3D12GraphicsCommandList> com;
  };

  // Helper struct used as a scope guard. Sets resource transition on construction and applies the oppposite one on destruction.
  struct ENGINE_API fresource_barrier_scope final
  {
    fresource_barrier_scope(fgraphics_command_list* in_command_list, ID3D12Resource* in_resource, D3D12_RESOURCE_STATES in_before, D3D12_RESOURCE_STATES in_after);
    ~fresource_barrier_scope();

    CTOR_MOVE_COPY_DELETE(fresource_barrier_scope)
    
    D3D12_RESOURCE_STATES before;
    D3D12_RESOURCE_STATES after;
    fgraphics_command_list* command_list = nullptr;
    ID3D12Resource* resource = nullptr;
  };
}
