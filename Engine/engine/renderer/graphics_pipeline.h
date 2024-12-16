#pragma once

#include <list>
#include <vector>

#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_core.h"

#include "core/com_pointer.h"
#include "engine/log.h"

struct IDxcBlob;
struct ID3D12RootSignature;
struct ID3D12PipelineState;

namespace engine
{
  struct fgraphics_pipeline
  {
    void reserve_parameters(uint32_t num);
    void add_constant_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t size, D3D12_SHADER_VISIBILITY visibility);
    void add_shader_resource_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    void add_constant_buffer_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    void add_unordered_access_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    void add_descriptor_table_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t num_descriptors, uint32_t offset_in_descriptors_from_table_start, D3D12_DESCRIPTOR_RANGE_TYPE range_type, D3D12_DESCRIPTOR_RANGE_FLAGS range_flags, D3D12_SHADER_VISIBILITY visibility);

    void add_static_sampler(uint32_t shader_register, D3D12_FILTER filter);

    void bind_pixel_shader(ComPtr<IDxcBlob>& shader);
    void bind_vertex_shader(ComPtr<IDxcBlob>& shader);
    void bind_command_list(ID3D12GraphicsCommandList* command_list);
    
    void setup_formats(uint32_t num_rtv_formats, const DXGI_FORMAT* rtv_formats, DXGI_FORMAT depth_buffer);
    void setup_input_layout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& in_input_layout);
    void setup_blend(uint32_t render_target_index, D3D12_BLEND source_blend, D3D12_BLEND destination_blend, D3D12_BLEND_OP blend_operation);

    void init(const char* name);

    DXGI_FORMAT get_depth_format() const
    {
      if(depth_buffer_format == DXGI_FORMAT_UNKNOWN)
      {
        LOG_ERROR("Invalid depth format")
      }
      return depth_buffer_format;
    }
    
    DXGI_FORMAT get_rtv_format(uint32_t index) const
    {
      DXGI_FORMAT format = render_target_formats.RTFormats[index];
      if(format == DXGI_FORMAT_UNKNOWN)
      {
        LOG_ERROR("Invalid rtv format")
      }
      return format;
    }
  
  private:
    std::vector<CD3DX12_ROOT_PARAMETER1> parameters;  // Keep in memory. This need to exist until root signature is created
    std::list<CD3DX12_DESCRIPTOR_RANGE1> ranges;      // Keep in memory. This need to exist until root signature is created
    std::vector<CD3DX12_STATIC_SAMPLER_DESC> static_samplers;
    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<ID3D12PipelineState> pipeline_state;
    ComPtr<IDxcBlob> vertex_shader;
    ComPtr<IDxcBlob> pixel_shader;
    DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_UNKNOWN;
    D3D12_RT_FORMAT_ARRAY render_target_formats{};
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;
    CD3DX12_BLEND_DESC blend_desc{D3D12_DEFAULT};
  };
}
