#include "descriptor_heap.h"

#include "d3d12.h"

#include "core/application.h"

namespace engine
{
  void fdescriptor::init(fdescriptor_heap* heap, uint32_t in_index)
  {
    parent_heap = heap;
    index = in_index;
    cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->com->GetCPUDescriptorHandleForHeapStart(), in_index, heap->increment_size);
    if(parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    {
      gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->com->GetGPUDescriptorHandleForHeapStart(), in_index, heap->increment_size);
    }
  }

  void fdescriptor_heap::push(fdescriptor& desc)
  {
    desc.init(this, next_index);
    descriptors.emplace_back(desc);
    next_index++;
  }

  fdescriptor* fdescriptor_heap::get(uint32_t index)
  {
    return &descriptors[index];
  }
}