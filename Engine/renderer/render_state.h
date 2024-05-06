#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace engine
{
    using Microsoft::WRL::ComPtr;
  
    struct ENGINE_API fstatic_mesh_render_state
    {
        ComPtr<ID3D11Buffer> vertex_buffer;
        unsigned int stride;
        unsigned int offset;
        
        ComPtr<ID3D11Buffer> index_buffer;
        unsigned int num_indices;
    };

    struct ENGINE_API ftexture_render_state
    {
        ComPtr<ID3D11ShaderResourceView> texture_srv;
    };

    struct ENGINE_API fpixel_shader_render_state
    {
        ComPtr<ID3D11PixelShader> shader;
        ComPtr<ID3D10Blob> shader_blob;
    };

    struct ENGINE_API fvertex_shader_render_state
    {
        ComPtr<ID3D11VertexShader> shader;
        ComPtr<ID3D10Blob> shader_blob;
    };
}