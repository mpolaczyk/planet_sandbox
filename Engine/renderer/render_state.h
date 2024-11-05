#pragma once

#include <vector>
#include <wrl/client.h>
#include "d3d12.h"
#include "dxcapi.h"

#include "math/vertex_data.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;

  struct fstatic_mesh_render_state
  {
    // Offline data
    std::vector<fvertex_data> vertex_list;
    std::vector<fface_data> face_list;
    
    // Present when resource is uploaded to GPU
    bool is_resource_online = false;
    ComPtr<ID3D12Resource> vertex_buffer;
    ComPtr<ID3D12Resource> index_buffer;
    ComPtr<ID3D12Resource> vertex_buffer_upload;  // TODO System resource, duplicates vertex_list. Clean vertex_list when this is set?
    ComPtr<ID3D12Resource> index_buffer_upload;   // TODO System resource, duplicates index_list. Clean index_list when this is set?
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
  };

  struct ftexture_render_state
  {
    // Offline data
    std::vector<float> data_hdr;
    std::vector<uint8_t> data_ldr;
    bool is_hdr;

    // Present when resource is uploaded to GPU
    bool is_resource_online = false;
    ComPtr<ID3D12Resource> texture_buffer;
    ComPtr<ID3D12Resource> texture_buffer_upload; // TODO System resource, duplicates data_*. Clean vertex_list when this is set?
  };

  struct fshader_render_state
  {
    ComPtr<IDxcBlob> blob;
  };
}
