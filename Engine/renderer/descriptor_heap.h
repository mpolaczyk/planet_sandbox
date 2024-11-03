#pragma once

#include <wrl/client.h>

#include "d3d12.h"
#include "d3dx12/d3dx12_root_signature.h"

#include <vector>


struct ID3D12DescriptorHeap;

namespace engine
{
  using namespace Microsoft::WRL;

  struct fdescriptor
  {
    fdescriptor(ID3D12DescriptorHeap* heap, uint32_t in_index, uint32_t descriptor_size, uint64_t in_resource_size);

    uint32_t index = 0;
    ComPtr<ID3D12Resource> resource;
    uint64_t resource_size = 0;
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle;
  };

  // Descriptor heap management. Keeps cpu/gpu handles, heap indexes, resources and resource sizes together.
  // No memory management, resource ownership, creation, upload etc.
  struct fdescriptor_heap
  {
    fdescriptor* push(uint64_t in_resource_size);
    fdescriptor* get(uint32_t index);

    ComPtr<ID3D12DescriptorHeap> heap;
    uint32_t descriptor_size = 0;
    
private:
    std::vector<fdescriptor> descriptors;
    uint32_t last_index = 0;
  };
}
