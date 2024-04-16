#pragma once

#include <wrl/client.h>

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"

struct ID3D11PixelShader;
struct ID3D10Blob;

namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  class ENGINE_API apixel_shader : public aasset_base
  {
  public:
    OBJECT_DECLARE(apixel_shader, aasset_base)
    OBJECT_DECLARE_LOAD(apixel_shader)
    OBJECT_DECLARE_SAVE(apixel_shader)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string shader_file_name;
    std::string entrypoint;
    std::string target;

    // Runtime
    ComPtr<ID3D11PixelShader> shader;
    ComPtr<ID3D10Blob> shader_blob;
  };
}