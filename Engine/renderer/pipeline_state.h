#pragma once

#include "d3dx12/d3dx12_pipeline_state_stream.h"

namespace engine
{
  struct fpipeline_state_stream
  {
    CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE root_signature;
    CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT input_layout;
    CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY primitive_topology_type;
    CD3DX12_PIPELINE_STATE_STREAM_VS vertex_shader;
    CD3DX12_PIPELINE_STATE_STREAM_PS pixel_shader;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT dsv_format;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtv_formats;
  };
}