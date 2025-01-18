#pragma once

#include <vector>

#include "core/com_pointer.h"
#include "engine/asset/soft_asset_ptr.h"
#include "engine/renderer/pipeline_type.h"
#include "engine/renderer/root_signature.h"

namespace engine
{
  class apixel_shader;
  class avertex_shader;
  class aray_tracing_shader;
  
  struct fpipeline
  {
    void setup_formats(uint32_t num_rtv_formats, const DXGI_FORMAT* rtv_formats, DXGI_FORMAT depth_buffer);
    void setup_blend(uint32_t render_target_index, D3D12_BLEND source_blend, D3D12_BLEND destination_blend, D3D12_BLEND_OP blend_operation);

    void add_static_sampler(uint32_t shader_register, D3D12_FILTER filter);

    void bind_pixel_shader(fsoft_asset_ptr<apixel_shader>& shader);
    void bind_vertex_shader(fsoft_asset_ptr<avertex_shader>& shader);
    void bind_ray_tracing_shader(fsoft_asset_ptr<aray_tracing_shader>& shader);
    void bind_command_list(ID3D12GraphicsCommandList* command_list);
    
    void setup_input_layout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& in_input_layout);

    void init(const char* name);

    DXGI_FORMAT get_depth_format() const;
    
    DXGI_FORMAT get_rtv_format(uint32_t index) const;

    epipeline_type type = epipeline_type::undefined;

    froot_signature root_signature_rasterization;
    froot_signature root_signature_ray_tracing_global;
    froot_signature root_signature_ray_tracing_local;
    
  private:

    std::vector<CD3DX12_STATIC_SAMPLER_DESC> static_samplers;

    // Rasterization
    fsoft_asset_ptr<apixel_shader> pixel_shader_asset;
    fsoft_asset_ptr<avertex_shader> vertex_shader_asset;
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;

    // Ray tracing
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_closestHitShaderName;
    static const wchar_t* c_missShaderName;
    fsoft_asset_ptr<aray_tracing_shader> ray_tracing_shader_asset;
    
    // Common
    fcom_ptr<ID3D12PipelineState> pipeline_state;
    D3D12_RT_FORMAT_ARRAY render_target_formats{};
    DXGI_FORMAT depth_buffer_format{};
    CD3DX12_BLEND_DESC blend_desc{D3D12_DEFAULT};
  };
}
