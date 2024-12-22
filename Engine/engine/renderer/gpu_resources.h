#pragma once

#include "core/com_pointer.h"
#include "dxcapi.h"   // IDxcBlob is possible to forward declare, but causes really strange compilation issues in client.h
//// It also needs ComPtr to be included first.

#include "engine/renderer/descriptor_heap.h"
#include "engine/renderer/dx12_lib.h"

struct IDxcBlob;
struct ID3D12Resource;
struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

namespace engine
{
  struct ENGINE_API fbuffer
  {
    fcom_ptr<ID3D12Resource> resource;
    uint32_t size = 0;

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
    void release();

    fcom_ptr<ID3D12Resource> com;
    fcom_ptr<ID3D12Resource> upload_com;
    fdescriptor srv;
    fdescriptor rtv;
    fdescriptor dsv;
  };

  struct ENGINE_API fshader_resource
  {
    fcom_ptr<IDxcBlob> blob;
  };

  struct ENGINE_API fstatic_mesh_resource
  {
    uint32_t vertex_num{};
    fcom_ptr<ID3D12Resource> vertex_buffer;
    fcom_ptr<ID3D12Resource> index_buffer;
    fcom_ptr<ID3D12Resource> vertex_buffer_upload;
    fcom_ptr<ID3D12Resource> index_buffer_upload;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};
    D3D12_INDEX_BUFFER_VIEW index_buffer_view{};
  };
}
