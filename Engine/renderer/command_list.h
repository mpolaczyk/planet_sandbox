#pragma once

#include <cstdint>
#include <wrl/client.h>

#include "core/core.h"

struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
enum D3D12_RESOURCE_STATES;

namespace engine
{
  class atexture;
  struct fstatic_mesh_render_state;
  using namespace Microsoft::WRL;
  
  struct ENGINE_API fgraphics_command_list
  {
    void resource_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const;

    void set_render_targets(ID3D12DescriptorHeap* dsv_descriptor_heap, ID3D12DescriptorHeap* rtv_descriptor_heap, int back_buffer_index) const;
    void set_viewport(uint32_t width, uint32_t height) const;
    void set_scissor(uint32_t width, uint32_t height) const;

    void clear_render_target(ID3D12DescriptorHeap* rtv_descriptor_heap, uint32_t back_buffer_index) const;
    void clear_depth_stencil(ID3D12DescriptorHeap* dsv_descriptor_heap) const;

    void upload_buffer_resource(uint64_t buffer_size, const void* in_buffer, ComPtr<ID3D12Resource>& out_upload_intermediate, ComPtr<ID3D12Resource>& out_gpu_resource) const;
    void upload_vertex_buffer(fstatic_mesh_render_state& out_render_state) const;
    void upload_index_buffer(fstatic_mesh_render_state& out_render_state) const;
    void upload_texture(atexture* texture_asset) const;
    
    ComPtr<ID3D12GraphicsCommandList> com; 
  };
}
