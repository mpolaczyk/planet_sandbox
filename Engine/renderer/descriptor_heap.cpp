#include "descriptor_heap.h"

#include "d3d12.h"

#include "core/application.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  fdescriptor::fdescriptor(ID3D12DescriptorHeap* heap, uint32_t in_index, uint32_t descriptor_size, uint64_t in_resource_size)
  {
    index = in_index;
    cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), in_index, descriptor_size);
    gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), in_index, descriptor_size);
    resource_size = in_resource_size;
  }

  fdescriptor* fdescriptor_heap::push(uint64_t in_resource_size)
  {
    fdescriptor temp(heap.Get(), last_index, descriptor_size, in_resource_size);
    temp.resource_size = in_resource_size;
    descriptors.emplace_back(temp);
    last_index++;
    return &descriptors.back(); 
  }

  fdescriptor* fdescriptor_heap::get(uint32_t index)
  {
    return &descriptors[index];
  }
}