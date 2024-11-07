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
    bool is_online = false;
    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12Resource> resource_upload;
    fdescriptor srv;
    
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t element_size;  // is_hdr ? sizeof(float) : sizeof(uint8_t)
    DXGI_FORMAT format;
    
    void upload(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, const void* data);
  };
  
}
