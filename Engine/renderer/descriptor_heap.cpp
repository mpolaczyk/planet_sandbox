#include "descriptor_heap.h"

#include "d3d12.h"

#include "core/application.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  fdescriptor::fdescriptor(fdescriptor_heap* heap, uint32_t in_index)
  {
    parent_heap = heap;
    index = in_index;
    cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->com->GetCPUDescriptorHandleForHeapStart(), in_index, heap->increment_size);
    gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->com->GetGPUDescriptorHandleForHeapStart(), in_index, heap->increment_size);
  }

  fdescriptor* fdescriptor_heap::push()
  {
    fdescriptor temp(this, last_index);
    descriptors.emplace_back(temp);
    last_index++;
    return &descriptors.back(); 
  }

  fdescriptor* fdescriptor_heap::get(uint32_t index)
  {
    return &descriptors[index];
  }
}