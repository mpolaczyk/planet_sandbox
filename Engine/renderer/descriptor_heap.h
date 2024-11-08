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
  
  struct ENGINE_API fdescriptor
  {
    fdescriptor() = default;
    fdescriptor(fdescriptor_heap* heap, uint32_t in_index);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle;

    fdescriptor_heap* parent_heap = nullptr;
    uint32_t index = 0;   // index in parent heap
  };
  
  struct ENGINE_API fdescriptor_heap
  {
    friend fdescriptor;

    fdescriptor_heap() = default;
    fdescriptor_heap(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE in_heap_type)
    {
      heap_type = in_heap_type;
      increment_size = device->GetDescriptorHandleIncrementSize(in_heap_type);
    }
    
    fdescriptor* push();
    fdescriptor* get(uint32_t index);

    ComPtr<ID3D12DescriptorHeap> com;
    
private:
    std::vector<fdescriptor> descriptors;
    uint32_t last_index = 0;
    uint32_t increment_size = 0;
    D3D12_DESCRIPTOR_HEAP_TYPE heap_type;
  };
}
