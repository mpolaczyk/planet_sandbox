#pragma once

#include <wrl/client.h>

#include "dxcapi.h"
#include "renderer/descriptor_heap.h"
#include "renderer/dx12_lib.h"

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
    CTOR_DEFAULT(ftexture_resource)
    CTOR_COPY_DELETE(ftexture_resource)   // Avoid copying to not increase the reference count, this makes cleanup harder
    CTOR_MOVE_DEFAULT(ftexture_resource)
    DTOR_DEFAULT(ftexture_resource)

    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12Resource> resource_upload;
    fdescriptor srv;

    void release()
    {
      srv.parent_heap->remove(srv.index);
      DX_RELEASE(resource)
      DX_RELEASE(resource_upload)
    }
  };

  struct ENGINE_API fdsv_resource
  {
    CTOR_DEFAULT(fdsv_resource)
    CTOR_COPY_DELETE(fdsv_resource)   // Avoid copying to not increase the reference count, this makes cleanup harder
    CTOR_MOVE_DEFAULT(fdsv_resource)
    DTOR_DEFAULT(fdsv_resource)

    ComPtr<ID3D12Resource> resource;
    fdescriptor dsv;
    
    void release()
    {
      dsv.parent_heap->remove(dsv.index);
      DX_RELEASE(resource)
    }
  };

  struct ENGINE_API frtv_resource
  {
    CTOR_DEFAULT(frtv_resource)
    CTOR_COPY_DELETE(frtv_resource)   // Avoid copying to not increase the reference count, this makes cleanup harder
    CTOR_MOVE_DEFAULT(frtv_resource)
    DTOR_DEFAULT(frtv_resource)

    ComPtr<ID3D12Resource> resource;
    fdescriptor rtv;

    void release()
    {
      rtv.parent_heap->remove(rtv.index);
      DX_RELEASE(resource)
    }
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
