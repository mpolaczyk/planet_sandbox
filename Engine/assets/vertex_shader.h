#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"

struct ID3D11VertexShader;

namespace engine
{

  class ENGINE_API vertex_shader_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(vertex_shader_asset, asset_base)
    OBJECT_DECLARE_LOAD(vertex_shader_asset)
    OBJECT_DECLARE_SAVE(vertex_shader_asset)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string shader_file_name;

    // Runtime
    ID3D11VertexShader* vertex_shader;
  };
}