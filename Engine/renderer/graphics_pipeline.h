#pragma once

#include <wrl/client.h>

struct ID3D12RootSignature;
struct ID3D12PipelineState;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fgraphics_pipeline
  {
    ComPtr<ID3D12RootSignature> signature;
    ComPtr<ID3D12PipelineState> state;
  };
}
