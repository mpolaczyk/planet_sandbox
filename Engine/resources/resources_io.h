#pragma once

#include <wrl/client.h>
#include <string>

#include "core/core.h"

struct ID3D10Blob;
struct IDxcBlob;

namespace engine
{
  using Microsoft::WRL::ComPtr;

  class astatic_mesh;
  class atexture;

  bool ENGINE_API load_obj(const std::string& file_name, astatic_mesh* out_static_mesh);

  bool ENGINE_API load_img(const std::string& file_name, atexture* out_texture);
  
  bool ENGINE_API load_hlsl_dxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, ComPtr<IDxcBlob>& out_shader_blob);
  
  //bool ENGINE_API load_hlsl_fxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, ComPtr<ID3D10Blob>& out_shader_blob);
}
