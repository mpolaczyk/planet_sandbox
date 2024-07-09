#pragma once

#include <vector>
#include <wrl/client.h>

#include "d3d12.h"
#include "d3dx12/d3dx12_core.h"
#include "math/vertex_data.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;

  struct fstatic_mesh_render_state
  {
    // Offline data
    std::vector<fvertex_data> vertex_list;
    unsigned int num_vertices;
    unsigned int vertex_list_size;
    unsigned int vertex_stride;
    std::vector<fface_data> face_list;
    unsigned int num_faces;
    unsigned int face_list_size;
    
    // Present when resource is uploaded to GPU
    bool is_resource_online = false;
    ComPtr<ID3D12Resource> vertex_buffer;
    ComPtr<ID3D12Resource> index_buffer;
    ComPtr<ID3D12Resource> vertex_buffer_upload;
    ComPtr<ID3D12Resource> index_buffer_upload;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
  };

  struct ftexture_render_state
  {
    //ComPtr<ID3D11Texture2D> texture;
    //ComPtr<ID3D11ShaderResourceView> texture_srv;
  };

  struct fpixel_shader_render_state
  {
    ComPtr<ID3DBlob> blob;
    //ComPtr<ID3D11PixelShader> shader;
    //ComPtr<ID3D10Blob> blob;
  };

  struct fvertex_shader_render_state
  {
    ComPtr<ID3DBlob> blob;
    //ComPtr<ID3D11VertexShader> shader;
    //ComPtr<ID3D10Blob> blob;
  };
}
