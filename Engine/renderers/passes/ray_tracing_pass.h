#pragma once

#include "engine/renderer/pass_base.h"
#include "engine/renderer/gpu_resources.h"

namespace engine
{
  struct fcommand_list;

  using namespace DirectX;
  
  struct SceneConstantBuffer
  {
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
  };

  struct CubeConstantBuffer
  {
    XMFLOAT4 albedo;
  };
  
  struct fray_tracing_pass : public fpass_base
  {
    virtual epipeline_type init_type() override { return epipeline_type::ray_tracing; }
    virtual void init_shaders() override;
    virtual void init_pipeline() override;
    virtual void init_size_independent_resources() override;
    virtual void init_size_dependent_resources(bool cleanup) override;
    virtual void draw(frenderer_context* in_context, fcommand_list* command_list) override;

    SceneConstantBuffer m_sceneCB[2];
    CubeConstantBuffer m_cubeCB;

    // Output
    ftexture_resource color;
  };
  
};
