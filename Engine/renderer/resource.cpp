#include "resource.h"

namespace engine
{

  void fresource::init(ID3D12DescriptorHeap* heap, int32_t index, uint32_t descriptor_size)
  {
    heap_index = index;
    cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), index, descriptor_size);
    gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), index, descriptor_size);
  }
}