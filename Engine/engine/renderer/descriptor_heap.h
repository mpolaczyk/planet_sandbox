#pragma once

#include <vector>

#include "d3dx12/d3dx12_root_signature.h"

#include "core/core.h"
#include "core/com_pointer.h"

#if BUILD_DEBUG
#include <string>
#endif

struct ID3D12DescriptorHeap;
enum D3D12_DESCRIPTOR_HEAP_TYPE;

namespace engine
{
  struct fdescriptor_heap;

  // Copyable descriptor. Holds reference to a heap and handles.
  // Does not own the ComPtr resource.
  struct ENGINE_API fdescriptor final
  {
    void init(fdescriptor_heap* heap, uint32_t in_index);
    void release();
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle{};
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_descriptor_handle{};

    fdescriptor_heap* parent_heap = nullptr;  // weak ptr, no ownership
    uint32_t index = -1;   // index in parent heap
#if BUILD_DEBUG
    std::string context;
#endif
  };

  // Heap management. Copyable.
  // Does not own the ComPtr resource.
  // Allows to:
  // - push to a heap
  // - remove usused descriptors
  // - reuse previously cleared slots
  struct ENGINE_API fdescriptor_heap final
  {
    friend fdescriptor;

    fdescriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE in_heap_type, uint32_t in_max_descriptors);
    CTOR_DEFAULT(fdescriptor_heap)
    CTOR_MOVE_COPY_DEFAULT(fdescriptor_heap)
    ~fdescriptor_heap();
    
    void push(fdescriptor& out_desc, const char* name);
    void remove(uint32_t index);
    fdescriptor* get(uint32_t index);
    void log_audit() const;
    
    fcom_ptr<ID3D12DescriptorHeap> com;
    
private:
    uint32_t find_free_index() const;
    
    std::vector<fdescriptor> descriptors;
    std::vector<bool> is_valid;             // index is descriptor index
    uint32_t max_descriptors{};
    uint32_t increment_size = 0;
    D3D12_DESCRIPTOR_HEAP_TYPE heap_type{};
  };
}
