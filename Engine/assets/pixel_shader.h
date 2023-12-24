#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"

struct ID3D11PixelShader;
struct ID3D10Blob;

namespace engine
{
  class ENGINE_API pixel_shader_asset : public asset_base
  {
  public:
    OBJECT_DECLARE(pixel_shader_asset, asset_base)
    OBJECT_DECLARE_LOAD(pixel_shader_asset)
    OBJECT_DECLARE_SAVE(pixel_shader_asset)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string shader_file_name;
    std::string entrypoint;
    std::string target;

    // Runtime
    ID3D11PixelShader* shader = nullptr;
    ID3D10Blob* shader_blob = nullptr;
  };
}