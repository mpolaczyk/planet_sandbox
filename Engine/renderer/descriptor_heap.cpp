#include "descriptor_heap.h"

#include "d3d12.h"

#include "core/application.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  fdescriptor::fdescriptor(fdescriptor_heap* heap, uint32_t in_index)
    : parent_heap(heap), index(in_index)
  {
    cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->com->GetCPUDescriptorHandleForHeapStart(), index, heap->increment_size);
    if(parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    {
      gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->com->GetGPUDescriptorHandleForHeapStart(), index, heap->increment_size);
    }
  }

  fdescriptor_heap::fdescriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE in_heap_type)
    : heap_type(in_heap_type)
  {
    increment_size = device->GetDescriptorHandleIncrementSize(in_heap_type);
  }

  void fdescriptor_heap::push(fdescriptor& desc)
  {
    desc = fdescriptor(this, next_index);
    descriptors.push_back(desc);
    next_index++;
  }

  fdescriptor* fdescriptor_heap::get(uint32_t index)
  {
    return &descriptors[index];
  }
}