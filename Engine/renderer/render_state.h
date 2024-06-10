#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace engine
{
  using Microsoft::WRL::ComPtr;

  struct fstatic_mesh_render_state
  {
    ComPtr<ID3D11Buffer> vertex_buffer;
    unsigned int stride;
    unsigned int offset;

    ComPtr<ID3D11Buffer> index_buffer;
    unsigned int num_indices;
  };

  struct ftexture_render_state
  {
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> texture_srv;
  };

  struct fpixel_shader_render_state
  {
    ComPtr<ID3D11PixelShader> shader;
    ComPtr<ID3D10Blob> shader_blob;
  };

  struct fvertex_shader_render_state
  {
    ComPtr<ID3D11VertexShader> shader;
    ComPtr<ID3D10Blob> shader_blob;
  };
}
