#pragma once

#include <cstdint>
#include <wrl/client.h>

#include "d3d12.h"
#include "d3dx12/d3dx12_root_signature.h"

namespace engine
{
  struct fresource
  {
    void init(ID3D12DescriptorHeap* heap, int32_t index, uint32_t descriptor_size);
    
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle;
    uint32_t heap_index;
  };
}
