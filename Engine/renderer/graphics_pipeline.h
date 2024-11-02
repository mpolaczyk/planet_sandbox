#pragma once

#include <list>
#include <wrl/client.h>

#include "dxcapi.h"
#include "d3dx12/d3dx12_root_signature.h"

#include <vector>

struct ID3D12RootSignature;
struct ID3D12PipelineState;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fgraphics_pipeline
  {
    void reserve_parameters(uint32_t num);
    void add_constant_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, size_t size, D3D12_SHADER_VISIBILITY visibility);
    void add_shader_respurce_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    void add_constant_buffer_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    void add_unordered_access_view_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    void add_descriptor_table_parameter(uint32_t index, uint32_t shader_register, uint32_t register_space, uint32_t num_descriptors, D3D12_DESCRIPTOR_RANGE_TYPE range_type, D3D12_DESCRIPTOR_RANGE_FLAGS range_flags, D3D12_SHADER_VISIBILITY visibility);

    void add_static_sampler(uint32_t shader_register, D3D12_FILTER filter);

    void bind_pixel_shader(ComPtr<IDxcBlob>& shader);
    void bind_vertex_shader(ComPtr<IDxcBlob>& shader);
    void bind_command_list(ID3D12GraphicsCommandList* command_list);
    
    void setup_formats(DXGI_FORMAT back_buffer, DXGI_FORMAT depth_buffer, const std::vector<DXGI_FORMAT>& render_targets);
    void setup_input_layout(std::vector<D3D12_INPUT_ELEMENT_DESC>&& in_input_layout);
    
    void init(const char* name);

  private:
    std::vector<CD3DX12_ROOT_PARAMETER1> parameters;
    std::list<CD3DX12_DESCRIPTOR_RANGE1> ranges;
    std::vector<CD3DX12_STATIC_SAMPLER_DESC> static_samplers;
    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<ID3D12PipelineState> pipeline_state;
    ComPtr<IDxcBlob> vertex_shader;
    ComPtr<IDxcBlob> pixel_shader;
    DXGI_FORMAT back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_D32_FLOAT;
    D3D12_RT_FORMAT_ARRAY render_target_formats = {};
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;
  };
}
