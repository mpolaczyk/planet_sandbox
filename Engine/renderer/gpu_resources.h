#pragma once

#include <wrl/client.h>

#include "d3d12.h"

#include "renderer/descriptor_heap.h"

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
}
