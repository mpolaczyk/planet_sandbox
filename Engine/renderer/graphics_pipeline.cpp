
#include "graphics_pipeline.h"

#include "d3dx12/d3dx12_core.h"

#include "core/application.h"
#include "math/math.h"
#include "renderer/device.h"
#include "renderer/pipeline_state.h"

// https://asawicki.info/news_1754_direct3d_12_long_way_to_access_data

namespace engine
{
  void fgraphics_pipeline::reserve_parameters(uint32_t num)
  {
    CD3DX12_ROOT_PARAMETER1 param = {};
    parameters.resize(num, param);
  }

  void fgraphics_pipeline::add_constant_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t size, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param;
    param.InitAsConstants(size / 4, shader_register, register_space);
    parameters[index] = param;
  }

  void fgraphics_pipeline::add_shader_resource_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param;
    param.InitAsShaderResourceView(shader_register, register_space, flags, visibility);
    parameters[index] = param;
  }

  void fgraphics_pipeline::add_constant_buffer_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param;
    param.InitAsConstantBufferView(shader_register, register_space, flags, visibility);
    parameters[index] = param;
  }

  void fgraphics_pipeline::add_unordered_access_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_ROOT_PARAMETER1 param;
    param.InitAsUnorderedAccessView(shader_register, register_space, flags, visibility);
    parameters[index] = param;
  }
  
  void fgraphics_pipeline::add_descriptor_table_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t num_descriptors, uint32_t offset_in_descriptors_from_table_start, D3D12_DESCRIPTOR_RANGE_TYPE range_type, D3D12_DESCRIPTOR_RANGE_FLAGS range_flags, D3D12_SHADER_VISIBILITY visibility)
  {
    if(offset_in_descriptors_from_table_start == 0)
    {
      offset_in_descriptors_from_table_start = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }
    CD3DX12_DESCRIPTOR_RANGE1 range(range_type, num_descriptors, shader_register, register_space, range_flags, offset_in_descriptors_from_table_start);
    ranges.push_back(range);
    
    CD3DX12_ROOT_PARAMETER1 param;
    //uint32_t num_descriptor_ranges = fmath::to_uint32(ranges.size());
    param.InitAsDescriptorTable(1, &ranges.back(), visibility);
    parameters[index] = param;
  }

  void fgraphics_pipeline::add_static_sampler(uint32_t shader_register, D3D12_FILTER filter)
  {
    CD3DX12_STATIC_SAMPLER_DESC desc(shader_register, filter);
    static_samplers.emplace_back(desc);
  }

  void fgraphics_pipeline::bind_pixel_shader(ComPtr<IDxcBlob>& shader)
  {
    pixel_shader = shader;  
  }

  void fgraphics_pipeline::bind_vertex_shader(ComPtr<IDxcBlob>& shader)
  {
    vertex_shader = shader;  
  }

  void fgraphics_pipeline::bind_command_list(ID3D12GraphicsCommandList* command_list)
  {
    command_list->SetGraphicsRootSignature(root_signature.Get());
    command_list->SetPipelineState(pipeline_state.Get());
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  }


  void fgraphics_pipeline::setup_formats(uint32_t num_rtv_formats, const DXGI_FORMAT* rtv_formats, DXGI_FORMAT depth_buffer)
  {
    if(num_rtv_formats > 8)
    {
      throw std::runtime_error("Maximum number of render targets exceeded.");
    }
    render_target_formats.NumRenderTargets = num_rtv_formats;
    for(uint32_t i = 0; i < render_target_formats.NumRenderTargets; i++)
    {
      render_target_formats.RTFormats[i] = rtv_formats[i];
    }
    depth_buffer_format = depth_buffer;
  }

  void fgraphics_pipeline::setup_input_layout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& in_input_layout)
  {
    input_layout = in_input_layout;
  }

  void fgraphics_pipeline::init(const char* name)
  {
    fdevice* device = fapplication::get_instance()->device.get();
    device->create_root_signature(parameters, static_samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, root_signature, name);

    fpipeline_state_stream pipeline_state_stream;
    pipeline_state_stream.root_signature = root_signature.Get();
    pipeline_state_stream.input_layout = { input_layout.data(), fmath::to_uint32(input_layout.size()) };
    pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipeline_state_stream.vertex_shader = CD3DX12_SHADER_BYTECODE(vertex_shader->GetBufferPointer(), vertex_shader->GetBufferSize());
    pipeline_state_stream.pixel_shader = CD3DX12_SHADER_BYTECODE(pixel_shader->GetBufferPointer(), pixel_shader->GetBufferSize());
    pipeline_state_stream.dsv_format = depth_buffer_format;
    pipeline_state_stream.rtv_formats = render_target_formats;
    device->create_pipeline_state(pipeline_state_stream, pipeline_state, name);
  }

}