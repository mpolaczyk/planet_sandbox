#pragma once

#include <wrl/client.h>

#include "dxcapi.h"
#include "renderer/descriptor_heap.h"

struct ID3D12Resource;
struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

namespace engine
{
  using namespace Microsoft::WRL;

  struct ENGINE_API fbuffer
  {
    ComPtr<ID3D12Resource> resource;
    uint64_t size = 0;

    void upload(const void* data) const;
  };

  struct ENGINE_API fconst_buffer : public fbuffer
  {
    fdescriptor cbv;
  };

  struct ENGINE_API fshader_resource_buffer : public fbuffer
  {
    fdescriptor srv;
  };
  
  struct ENGINE_API ftexture_resource
  {
    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12Resource> resource_upload;
    fdescriptor srv;
  };

  struct ENGINE_API fdsv_resource
  {
    ComPtr<ID3D12Resource> resource;
    fdescriptor desc;
  };
  
  struct ENGINE_API fshader_resource
  {
    ComPtr<IDxcBlob> blob;
  };

  struct ENGINE_API fstatic_mesh_resource
  {
    uint32_t vertex_num;
    ComPtr<ID3D12Resource> vertex_buffer;
    ComPtr<ID3D12Resource> index_buffer;
    ComPtr<ID3D12Resource> vertex_buffer_upload;
    ComPtr<ID3D12Resource> index_buffer_upload;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
  };
}
