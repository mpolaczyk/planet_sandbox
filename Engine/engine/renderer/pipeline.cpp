#include "stdafx.h"

#include "pipeline.h"

#include "assets/vertex_shader.h"
#include "assets/pixel_shader.h"
#include "engine/math/math.h"
#include "engine/renderer/device.h"
#include "engine/renderer/pipeline_state.h"
#include "engine/resources/shader_tools.h"

// https://asawicki.info/news_1754_direct3d_12_long_way_to_access_data

namespace engine
{
  void fpipeline::setup_formats(uint32_t num_rtv_formats, const DXGI_FORMAT* rtv_formats, DXGI_FORMAT depth_buffer)
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

  void fpipeline::setup_blend(uint32_t render_target_index, D3D12_BLEND source_blend, D3D12_BLEND destination_blend, D3D12_BLEND_OP blend_operation)
  {
    blend_desc.RenderTarget[render_target_index].BlendEnable = true;
    blend_desc.RenderTarget[render_target_index].SrcBlend = source_blend;
    blend_desc.RenderTarget[render_target_index].DestBlend = destination_blend;
    blend_desc.RenderTarget[render_target_index].BlendOp = blend_operation;
  }

  
  void fpipeline::add_static_sampler(uint32_t shader_register, D3D12_FILTER filter)
  {
    CD3DX12_STATIC_SAMPLER_DESC desc(shader_register, filter);
    static_samplers.emplace_back(desc);
  }

  void fpipeline::bind_pixel_shader(fsoft_asset_ptr<apixel_shader>& shader)
  {
    pixel_shader_asset = shader;
  }

  void fpipeline::bind_vertex_shader(fsoft_asset_ptr<avertex_shader>& shader)
  {
    vertex_shader_asset = shader;
  }

  void fpipeline::bind_command_list(ID3D12GraphicsCommandList* command_list)
  {
    if(type == epipeline_type::rasterization)
    {
      command_list->SetGraphicsRootSignature(root_signature_rasterization.com.Get());
      command_list->SetPipelineState(pipeline_state.Get());
      command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
    else if(type == epipeline_type::ray_tracing)
    {
      // TODO
    }
  }

  void fpipeline::setup_input_layout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& in_input_layout)
  {
    input_layout = in_input_layout;
  }

  

  void fpipeline::init(const char* name)
  {
    fdevice* device = fapplication::get_instance()->device.get();

    if(type == epipeline_type::rasterization)
    {
      device->create_root_signature(root_signature_rasterization.parameters, static_samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, root_signature_rasterization.com, name);

      fraster_pipeline_state_stream pipeline_state_stream;
      pipeline_state_stream.root_signature = root_signature_rasterization.com.Get();
      pipeline_state_stream.input_layout = { input_layout.data(), fmath::to_uint32(input_layout.size()) };
      pipeline_state_stream.primitive_topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      pipeline_state_stream.pixel_shader = fshader_tools::get_shader_byte_code(pixel_shader_asset.get()->resource.blob.Get());
      pipeline_state_stream.vertex_shader = fshader_tools::get_shader_byte_code(vertex_shader_asset.get()->resource.blob.Get());
      pipeline_state_stream.depth_stencil_format = depth_buffer_format;
      pipeline_state_stream.render_target_formats = render_target_formats;
      pipeline_state_stream.blend_desc = blend_desc;
      device->create_pipeline_state(pipeline_state_stream, pipeline_state, name);
    }
    else if(type == epipeline_type::ray_tracing)
    {
      device->create_root_signature(root_signature_ray_tracing_global.parameters, static_samplers, D3D12_ROOT_SIGNATURE_FLAG_NONE, root_signature_ray_tracing_global.com, name);

      device->create_root_signature(root_signature_ray_tracing_local.parameters, static_samplers, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE, root_signature_ray_tracing_local.com, name);
    }
  }

  DXGI_FORMAT fpipeline::get_depth_format() const
  {
    if (depth_buffer_format == DXGI_FORMAT_UNKNOWN)
    {
      LOG_ERROR("Invalid depth format")
    }
    return depth_buffer_format;
  }

  DXGI_FORMAT fpipeline::get_rtv_format(uint32_t index) const
  {
    DXGI_FORMAT format = render_target_formats.RTFormats[index];
    if (format == DXGI_FORMAT_UNKNOWN)
    {
      LOG_ERROR("Invalid rtv format")
    }
    return format;
  }
}