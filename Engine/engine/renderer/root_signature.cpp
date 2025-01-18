#include "stdafx.h"

#include "engine/renderer/root_signature.h"

namespace engine
{
  void froot_signature::reserve_parameters(uint32_t num)
  {
    CD3DX12_ROOT_PARAMETER1 param{};
    parameters.resize(num, param);
  }

  void froot_signature::add_constant_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t size, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param{};
    param.InitAsConstants(size / 4, shader_register, register_space);
    parameters[index] = param;
  }

  void froot_signature::add_shader_resource_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param{};
    param.InitAsShaderResourceView(shader_register, register_space, flags, visibility);
    parameters[index] = param;
  }

  void froot_signature::add_constant_buffer_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param{};
    param.InitAsConstantBufferView(shader_register, register_space, flags, visibility);
    parameters[index] = param;
  }

  void froot_signature::add_unordered_access_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param{};
    param.InitAsUnorderedAccessView(shader_register, register_space, flags, visibility);
    parameters[index] = param;
  }
  
  void froot_signature::add_descriptor_table_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t num_descriptors, uint32_t offset_in_descriptors_from_table_start, D3D12_DESCRIPTOR_RANGE_TYPE range_type, D3D12_DESCRIPTOR_RANGE_FLAGS range_flags, D3D12_SHADER_VISIBILITY visibility)
  {
    if(offset_in_descriptors_from_table_start == 0)
    {
      offset_in_descriptors_from_table_start = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }
    CD3DX12_DESCRIPTOR_RANGE1 range(range_type, num_descriptors, shader_register, register_space, range_flags, offset_in_descriptors_from_table_start);
    ranges.push_back(range);
    
    CD3DX12_ROOT_PARAMETER1 param{};
    //uint32_t num_descriptor_ranges = fmath::to_uint32(ranges.size());
    param.InitAsDescriptorTable(1, &ranges.back(), visibility);
    parameters[index] = param;
  }
}