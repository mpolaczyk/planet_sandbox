#pragma once

#include <wrl/client.h>
#include <string>

#include "core/core.h"

struct ID3D10Blob;
struct IDxcBlob;

namespace engine
{
  using Microsoft::WRL::ComPtr;

  bool ENGINE_API load_shader_cache(const std::string& file_name, ComPtr<IDxcBlob>& out_shader_blob);
  
  bool ENGINE_API load_hlsl_dxc(const std::string& hlsl_file_name, const std::string& entrypoint, const std::string& target, ComPtr<IDxcBlob>& out_shader_blob, std::string& out_has);

#if USE_FXC
  bool ENGINE_API load_hlsl_fxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, ComPtr<ID3D10Blob>& out_shader_blob);
#endif
}
