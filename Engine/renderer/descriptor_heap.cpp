#include "descriptor_heap.h"

#include "d3d12.h"

#include "core/application.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  fdescriptor::fdescriptor(fdescriptor_heap* heap, uint32_t in_index)
    : parent_heap(heap), index(in_index)
  {
    cpu_descriptor_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->com->GetCPUDescriptorHandleForHeapStart(), index, heap->increment_size);
    if(parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    {
      // Only for heap of a type: D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
      gpu_descriptor_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->com->GetGPUDescriptorHandleForHeapStart(), index, heap->increment_size);
    }
  }

  fdescriptor::~fdescriptor()
  {
    release();
  }

  void fdescriptor::release()
  {
    if(parent_heap)
    {
      parent_heap->remove(index);
    }
  }

  fdescriptor_heap::fdescriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE in_heap_type, uint32_t in_max_descriptors)
    : heap_type(in_heap_type), max_descriptors(in_max_descriptors)
  {
    increment_size = device->GetDescriptorHandleIncrementSize(in_heap_type);
    fdescriptor temp;
    descriptors.resize(in_max_descriptors, temp);
    bool temp2 = false;
    is_valid.resize(in_max_descriptors, temp2);
  }

  void fdescriptor_heap::push(fdescriptor& out_desc)
  {
    uint32_t index = find_free_index();
    out_desc = fdescriptor(this, index);
    descriptors[index] = out_desc;
    is_valid[index] = true;
  }

  void fdescriptor_heap::remove(uint32_t index)
  {
    if(is_valid[index])
    {
      descriptors[index] = fdescriptor();
      is_valid[index] = false;
    }
  }

  fdescriptor* fdescriptor_heap::get(uint32_t index)
  {
    return &descriptors[index];
  }

  uint32_t fdescriptor_heap::find_free_index() const
  {
    // TODO Linear search is ok for now. Switch to a ring buffer later
    for(uint32_t i = 0; i < is_valid.size(); i++)
    {
      if(!is_valid[i])
      {
        return i;
      }
    }
    throw std::runtime_error("Can't push more descriptors on a heap");
  }
}