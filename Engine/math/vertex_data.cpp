
#include "math/vertex_data.h"

namespace engine
{
  std::vector<D3D12_INPUT_ELEMENT_DESC> fvertex_data::input_layout =
    {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

  const std::vector<fvertex_data>& fvertex_data::get_quad_vertex_list()
  {
    static fvertex_data v0({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f, 0.f}, {0.0f, 1.0f});
    static fvertex_data v1({-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f, 0.f}, {0.0f, 0.0f});
    static fvertex_data v2({1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f, 0.f}, {1.0f, 0.0f});
    static fvertex_data v3({1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f, 0.f}, {1.0f, 1.0f});
    static std::vector<fvertex_data> vertex_list = {v0,v1,v2,v3};
    return vertex_list;
  }

  const std::vector<fface_data>& fface_data::get_quad_face_list()
  {
    static fface_data f0{0, 1, 2};
    static fface_data f1{0, 2, 3};
    static std::vector<fface_data> face_list = {f0,f1};
    return face_list;
  }
}