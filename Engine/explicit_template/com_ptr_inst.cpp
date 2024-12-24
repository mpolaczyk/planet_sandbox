#include "core/com_pointer.h"

#include "d3d12.h"

namespace engine
{
  template fcom_ptr<ID3D12Device2>;
  template fcom_ptr<ID3D12DescriptorHeap>;
  template fcom_ptr<ID3D12CommandQueue>;
  template fcom_ptr<ID3D12GraphicsCommandList>;
}