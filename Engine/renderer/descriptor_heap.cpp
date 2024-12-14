#include "descriptor_heap.h"

#include "d3d12.h"

#include "core/application.h"
#include "engine/log.h"

namespace engine
{
  void fdescriptor::init(fdescriptor_heap* heap, uint32_t in_index)
  {
    index = in_index;
    parent_heap = heap;
    cpu_descriptor_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->com->GetCPUDescriptorHandleForHeapStart(), index, heap->increment_size);
    if(parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && parent_heap->heap_type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    {
      // Only for heap of a type: D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
      gpu_descriptor_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->com->GetGPUDescriptorHandleForHeapStart(), index, heap->increment_size);
    }
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

  fdescriptor_heap::~fdescriptor_heap()
  {
    for(uint32_t i = 0; i < is_valid.size(); i++)
    {
      remove(i);
    }
  }

  void fdescriptor_heap::push(fdescriptor& out_desc, const char* name)
  {
    uint32_t index = find_free_index();
    out_desc.init(this, index);
#if BUILD_DEBUG
    out_desc.context = name;
#endif
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

  void fdescriptor_heap::log_audit() const
  {
    LOG_INFO("### Auditing heap: {} ###", static_cast<int>(heap_type))
    LOG_INFO("Increment size {} Max descriptors {}", increment_size, max_descriptors);
    for(uint32_t i = 0; i < is_valid.size(); i++)
    {
      if(is_valid[i])
      {
        const fdescriptor& d = descriptors[i];
#if BUILD_DEBUG
        LOG_INFO("Index {}:{}  GPU {}  CPU {}  Context {}", i, d.index, d.gpu_descriptor_handle.ptr, d.cpu_descriptor_handle.ptr, d.context);
#elif BUILD_RELEASE
        LOG_INFO("Index {}:{}  GPU {}  CPU {}", i, d.index, d.gpu_descriptor_handle.ptr, d.cpu_descriptor_handle.ptr);
#endif
      }
    }
  }

  uint32_t fdescriptor_heap::find_free_index() const
  {
    // TODO Linear search is ok for now. Switch to sth better later
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

  