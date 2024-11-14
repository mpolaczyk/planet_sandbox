#pragma once

#include <wrl/client.h>
#include <vector>

#include "d3d12.h"
#include "d3dx12/d3dx12_root_signature.h"

#include "core/core.h"

struct ID3D12DescriptorHeap;

namespace engine
{
  using namespace Microsoft::WRL;

  struct fdescriptor_heap;
  
  struct ENGINE_API fdescriptor final
  {
    CTOR_DEFAULT(fdescriptor)
    CTOR_MOVE_COPY_DEFAULT(fdescriptor)
    DTOR_DEFAULT(fdescriptor)
    fdescriptor(fdescriptor_heap* heap, uint32_t in_index);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle;

    fdescriptor_heap* parent_heap = nullptr;  // weak ptr, no ownership
    uint32_t index = -1;   // index in parent heap
  };
  
  struct ENGINE_API fdescriptor_heap final
  {
    friend fdescriptor;

    CTOR_DEFAULT(fdescriptor_heap)
    CTOR_MOVE_COPY_DEFAULT(fdescriptor_heap)
    fdescriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE in_heap_type);

    void push(fdescriptor& desc);

    fdescriptor* get(uint32_t index);

    ComPtr<ID3D12DescriptorHeap> com;
    
private:
    std::vector<fdescriptor> descriptors;
    uint32_t next_index = 0;
    uint32_t increment_size = 0;
    D3D12_DESCRIPTOR_HEAP_TYPE heap_type;
  };
}
