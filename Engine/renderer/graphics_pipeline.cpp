
#include "graphics_pipeline.h"

#include "d3dx12/d3dx12_core.h"

#include "core/application.h"
#include "renderer/dx12_lib.h"
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

  void fgraphics_pipeline::add_shader_respurce_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility)
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
  
  void fgraphics_pipeline::add_descriptor_table_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t num_descriptors, D3D12_DESCRIPTOR_RANGE_TYPE range_type, D3D12_DESCRIPTOR_RANGE_FLAGS range_flags, D3D12_SHADER_VISIBILITY visibility)
  {
    CD3DX12_DESCRIPTOR_RANGE1 range(range_type, num_descriptors, shader_register, register_space, range_flags, 0);
    ranges.push_back(range);
    
    CD3DX12_ROOT_PARAMETER1 param;
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


  void fgraphics_pipeline::setup_formats(DXGI_FORMAT back_buffer, DXGI_FORMAT depth_buffer, const std::vector<DXGI_FORMAT>& render_targets)
  {
    back_buffer_format = back_buffer;
    depth_buffer_format = depth_buffer;
    render_target_formats.NumRenderTargets = min(8, static_cast<uint32_t>(render_targets.size()));
    for(uint32_t i = 0; i < render_target_formats.NumRenderTargets; i++)
    {
      render_target_formats.RTFormats[i] = render_targets[i];
    }
  }

  void fgraphics_pipeline::setup_input_layout(std::vector<D3D12_INPUT_ELEMENT_DESC>&& in_input_layout)
  {
    input_layout = std::move(in_input_layout);
  }

  void fgraphics_pipeline::init(const char* name)
  {
    fdevice& device = fapplication::instance->device;

    fdx12::create_root_signature(device.com, parameters, static_samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, root_signature);
#if BUILD_DEBUG
    DX_SET_NAME(root_signature, "Root signature: {0}", name)
#endif

    fpipeline_state_stream pipeline_state_stream;
    pipeline_state_stream.root_signature = root_signature.Get();
    pipeline_state_stream.input_layout = { input_layout.data(), static_cast<uint32_t>(input_layout.size()) };
    pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipeline_state_stream.vertex_shader = CD3DX12_SHADER_BYTECODE(vertex_shader->GetBufferPointer(), vertex_shader->GetBufferSize());
    pipeline_state_stream.pixel_shader = CD3DX12_SHADER_BYTECODE(pixel_shader->GetBufferPointer(), pixel_shader->GetBufferSize());
    pipeline_state_stream.dsv_format = depth_buffer_format;
    pipeline_state_stream.rtv_formats = render_target_formats;
    fdx12::create_pipeline_state(device.com, pipeline_state_stream, pipeline_state);
#if BUILD_DEBUG
    DX_SET_NAME(pipeline_state, "Pipeline state: {0}", name)
#endif
  }

}