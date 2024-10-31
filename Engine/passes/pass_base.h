#pragma once

#include <wrl/client.h>

#include "assets/material.h"

struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct frenderer_context;
  
  struct fpass_base
  {
    virtual ~fpass_base() = default;
    
    virtual void init() = 0;
    virtual void draw(ComPtr<ID3D12GraphicsCommandList> command_list) = 0;
    virtual void create_output_texture(bool cleanup = false) = 0;
    virtual void set_renderer_context(frenderer_context* in_context)
    {
      context = in_context;
    }
    bool get_can_draw() const { return can_draw; }
    
  protected:
    
    // Input
    frenderer_context* context = nullptr; // weak ptr, owned by renderer
    
    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<ID3D12PipelineState> pipeline_state;

    bool can_draw = true;
  };
}